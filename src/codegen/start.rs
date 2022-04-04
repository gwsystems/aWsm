use crate::codegen::ModuleCtx;
use crate::wasm::Function::Implemented;
use llvm::Builder;
use llvm::FunctionType;
use llvm::Sub;

use crate::llvm::Compile;

// The start component of a module declares the function index of a start function that is automatically invoked
// when the module is instantiated, after tables and memories have been initialized.
// https://webassembly.github.io/spec/core/syntax/modules.html#start-function

pub const START_INSTRUCTION: &str = "awsm_abi__start_fn";

pub fn generate_start_stub(ctx: &ModuleCtx, start_fn_idx: Option<u32>) {
    if let Some(idx) = start_fn_idx {
        let (_, func) = &ctx.functions[idx as usize];

        match func {
            Implemented { f } => {
                if let Some(start_function_type) = &f.ty {
                    if start_function_type.params.len() > 0 {
                        panic!("Expected start function to not take parameters");
                    }
                } else {
                    panic!("Expected start function to have type");
                }

                let setup_function = ctx.llvm_module.add_function(
                    START_INSTRUCTION,
                    FunctionType::new(<()>::get_type(ctx.llvm_ctx), &[]).to_super(),
                );
                let bb = setup_function.append("entry");
                let b = Builder::new(ctx.llvm_ctx);
                b.position_at_end(bb);
                b.build_call(
                    ctx.llvm_module.get_function(&f.generated_name).unwrap(),
                    &[],
                );

                b.build_ret_void();
            }
            _ => {
                panic!("Expected start function to be locally implemented");
            }
        }
    } else {
        let setup_function = ctx.llvm_module.add_function(
            START_INSTRUCTION,
            FunctionType::new(<()>::get_type(ctx.llvm_ctx), &[]).to_super(),
        );
        let bb = setup_function.append("entry");
        let b = Builder::new(ctx.llvm_ctx);
        b.position_at_end(bb);
        b.build_ret_void();
    }
}
