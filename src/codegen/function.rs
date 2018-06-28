use std::cell::Cell;

use llvm::BasicBlock;
use llvm::Builder;
use llvm::Function;
use llvm::Value;

use super::super::wasm::ImplementedFunction;

use super::block::compile_block;
use super::breakout::BreakoutTarget;
use super::type_conversions::wasm_type_to_zeroed_value;
use super::ModuleCtx;

pub struct FunctionCtx<'a> {
    pub llvm_f: &'a Function,
    pub builder: &'a Builder,
    pub has_return: bool,
    block_counter: Cell<u32>,
}

impl<'a> FunctionCtx<'a> {
    pub fn generate_block(&self) -> &BasicBlock {
        let n = self.block_counter.get();
        let result = self.llvm_f.append(&format!("b_{}", n));
        self.block_counter.set(n + 1);
        result
    }
}

pub fn compile_function(ctx: &ModuleCtx, f: &ImplementedFunction) {
    //    println!("function {}", f.generated_name);

    let llvm_f = ctx.llvm_module.get_function(&f.generated_name).unwrap();

    let mut locals: Vec<&Value> = Vec::new();
    // In WASM, locals start with function arguments
    for i in 0..llvm_f.get_signature().get_params().len() {
        locals.push(&*llvm_f[i]);
    }
    // Then the actual locals
    for local in &f.locals {
        locals.push(wasm_type_to_zeroed_value(ctx.llvm_ctx, *local));
    }

    let builder = &*Builder::new(ctx.llvm_ctx);

    let initial_bb = llvm_f.append("entry");

    let f_ctx = FunctionCtx {
        llvm_f,
        builder,
        has_return: f.has_return(),
        block_counter: Cell::new(0),
    };

    let termination_block = llvm_f.append("exit");
    let root_breakout_target = BreakoutTarget::new_wrapped(termination_block, f.get_return_type());

    // In WASM, a break out of the root block is the same as returning from the function
    // Thus the termination block needs to do that
    let mut breakout_stack = vec![root_breakout_target.clone()];

    // We compile the root level as if it's a block
    compile_block(
        ctx,
        &f_ctx,
        &mut breakout_stack,
        &root_breakout_target.clone(),
        locals,
        initial_bb,
        f.code.as_slice(),
    );

    // The termination block just returns the value yielded by the top level block
    let result = root_breakout_target
        .borrow()
        .build_result(ctx.llvm_ctx, builder);
    builder.position_at_end(termination_block);
    match result {
        Some(v) => builder.build_ret(v),
        None => builder.build_ret_void(),
    };
}

// NOTE: Handle loop at the call site by inserting phi instructions immedietly
// That means the recursive call doesn't need to care about what type it is
