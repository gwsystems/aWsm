use std::cell::Cell;
#[cfg(feature = "cortex-m-attrs")]
use std::ffi::CString;
#[cfg(feature = "cortex-m-attrs")]
use std::mem;

use llvm::BasicBlock;
#[cfg(feature = "cortex-m-attrs")]
use llvm::Sub;
use llvm::Builder;
use llvm::Function;
use llvm::Value;

use super::super::wasm::ImplementedFunction;

use super::block::compile_block;
use super::breakout::BreakoutTarget;
use super::type_conversions::wasm_type_to_zeroed_value;
use super::ModuleCtx;
use wasmparser::TypeOrFuncType;

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

#[cfg(feature = "cortex-m-attrs")]
unsafe fn add_string_attr(k: &str, v: &str, v_ref: *mut llvm_sys::LLVMValue, ctx: &ModuleCtx) {
    let idx = llvm_sys::LLVMAttributeFunctionIndex;

    let target_features_key = CString::new(k).unwrap();
    let target_features_value = CString::new(v).unwrap();

    let llvm_ctx: *mut llvm_sys::LLVMContext = mem::transmute(ctx.llvm_ctx as &llvm::Context);
    let attr_ref = llvm_sys::core::LLVMCreateStringAttribute(
        llvm_ctx,
        target_features_key.as_ptr(),
        target_features_key.as_bytes().len() as u32,
        target_features_value.as_ptr(),
        target_features_value.as_bytes().len() as u32,
    );

    llvm_sys::core::LLVMAddAttributeAtIndex(
        v_ref,
        idx,
        attr_ref
    );

}

#[cfg(feature = "cortex-m-attrs")]
fn add_cortex_m_attrs(llvm_f: &Function, ctx: &ModuleCtx) {
    // FIXME: This is a performance hack to include target features for cortex-m
    // The version of clang we're using for the cortex-m work needs these (at least for lto),
    // otherwise it falls back to slow defaults
    //
    // attributes #0 = {
    // nounwind
    // "correctly-rounded-divide-sqrt-fp-math"="false"
    // "disable-tail-calls"="false"
    // "less-precise-fpmad"="false"
    // "min-legal-vector-width"="0"
    // "no-frame-pointer-elim"="true"
    // "no-frame-pointer-elim-non-leaf"
    // "no-infs-fp-math"="false"
    // "no-jump-tables"="false"
    // "no-nans-fp-math"="false"
    // "no-signed-zeros-fp-math"="false"
    // "no-trapping-math"="false"
    // "stack-protector-buffer-size"="8"
    // "target-cpu"="cortex-m7"
    // "target-features"="+armv7e-m,+dsp,+fp-armv8d16,+fp-armv8d16sp,+fp16,+fp64,+fpregs,+hwdiv,+thumb-mode,+vfp2,+vfp2d16,+vfp2d16sp,+vfp2sp,+vfp3d16,+vfp3d16sp,+vfp4d16,+vfp4d16sp,-aes,-crc,-crypto,-dotprod,-fp16fml,-fullfp16,-hwdiv-arm,-lob,-mve,-mve.fp,-ras,-sb,-sha2"
    // "unsafe-fp-math"="false"
    // "use-soft-float"="false" }
    unsafe {
        let v_ref: *mut llvm_sys::LLVMValue = mem::transmute(llvm_f.to_super() as &Value);
        let llvm_ctx: *mut llvm_sys::LLVMContext = mem::transmute(ctx.llvm_ctx as &llvm::Context);

        let nounwind = CString::new("nounwind").unwrap();
        let kind = llvm_sys::core::LLVMGetEnumAttributeKindForName(nounwind.as_ptr(), nounwind.as_bytes().len());
        let attr_ref = llvm_sys::core::LLVMCreateEnumAttribute(llvm_ctx, kind, 0);

        llvm_sys::core::LLVMAddAttributeAtIndex(
            v_ref,
            llvm_sys::LLVMAttributeFunctionIndex,
            attr_ref
        );

        info!("engaging cortex-m override");

        add_string_attr(
            "correctly-rounded-divide-sqrt-fp-math",
            "false",
            v_ref,
            ctx
        );

        add_string_attr(
            "disable-tail-calls",
            "false",
            v_ref,
            ctx
        );

        add_string_attr(
            "less-precise-fpmad",
            "false",
            v_ref,
            ctx
        );

        add_string_attr(
            "min-legal-vector-width",
            "0",
            v_ref,
            ctx
        );

        add_string_attr(
            "no-frame-pointer-elim",
            "false",
            v_ref,
            ctx
        );

        add_string_attr(
            "no-infs-fp-math",
            "false",
            v_ref,
            ctx
        );

        add_string_attr(
            "no-jump-tables",
            "false",
            v_ref,
            ctx
        );

        add_string_attr(
            "no-nans-fp-math",
            "false",
            v_ref,
            ctx
        );

        add_string_attr(
            "no-signed-zeros-fp-math",
            "false",
            v_ref,
            ctx
        );

        add_string_attr(
            "no-trapping-math",
            "false",
            v_ref,
            ctx
        );

        // add_string_attr(
        //     "stack-protector-buffer-size",
        //     "8",
        //     v_ref,
        //     ctx
        // );

        add_string_attr(
            "target-cpu",
            "cortex-m7",
            v_ref,
            ctx
        );

        add_string_attr(
            "unsafe-fp-math",
            "false",
            v_ref,
            ctx
        );

        add_string_attr(
            "use-soft-float",
            "false",
            v_ref,
            ctx
        );


        add_string_attr(
            "target-features",
            "+armv7e-m,+dsp,+fp-armv8d16,+fp-armv8d16sp,+fp16,+fp64,+fpregs,+hwdiv,+thumb-mode,+vfp2,+vfp2d16,+vfp2d16sp,+vfp2sp,+vfp3d16,+vfp3d16sp,+vfp4d16,+vfp4d16sp,-aes,-crc,-crypto,-dotprod,-fp16fml,-fullfp16,-hwdiv-arm,-lob,-mve,-mve.fp,-ras,-sb,-sha2",
            v_ref,
            ctx
        );
    }
}

pub fn compile_function(ctx: &ModuleCtx, f: &ImplementedFunction) {
    let llvm_f = ctx.llvm_module.get_function(&f.generated_name).unwrap();

    // Add Cortex-M attributes?
    #[cfg(feature = "cortex-m-attrs")]
    if let Some(target) = &ctx.opt.target {
        if target.contains("thumbv7em-none-unknown-eabi") {
            add_cortex_m_attrs(&llvm_f, ctx);
        }
    }

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
    let root_breakout_target = BreakoutTarget::new_wrapped(termination_block, f.get_return_type().map(TypeOrFuncType::Type));

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

// NOTE: Handle loop at the call site by inserting phi instructions immediately
// That means the recursive call doesn't need to care about what type it is
