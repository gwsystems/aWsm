use llvm::Compile;
use llvm::Context as LLVMCtx;
use llvm::Module as LLVMModule;
use llvm::Sub;
use llvm::Value;

use super::super::wasm::Global;
use super::super::wasm::Instruction;

use super::type_conversions::llvm_type_to_wasm_type;
use super::type_conversions::wasm_type_to_llvm_type;

pub fn insert_global<'a>(
    llvm_ctx: &'a LLVMCtx,
    llvm_module: &'a LLVMModule,
    g: Global,
) -> &'a Value {
    match g {
        Global::Imported {
            name,
            content_type,
            mutable,
        } => {
            let llvm_global =
                llvm_module.add_global(&name, wasm_type_to_llvm_type(&*llvm_ctx, content_type));
            llvm_global.set_constant(!mutable);
            llvm_global.to_super()
        }
        Global::InModule {
            generated_name,
            content_type,
            mutable,
            initializer,
        } => {
            assert_eq!(initializer.len(), 1);
            let v = match initializer[0] {
                Instruction::I32Const(i) => i.compile(llvm_ctx),
                Instruction::I64Const(i) => i.compile(llvm_ctx),
                Instruction::F32Const(f) => f.compile(llvm_ctx),
                Instruction::F64Const(f) => f.compile(llvm_ctx),
                ref e => panic!("Non simple initializer instruction {:?}", e),
            };
            let ty = llvm_type_to_wasm_type(llvm_ctx, v.get_type());
            assert_eq!(ty, content_type);

            let llvm_global = llvm_module.add_global_variable(&generated_name, v);
            llvm_global.set_constant(!mutable);
            llvm_global.to_super()
        }
    }
}
