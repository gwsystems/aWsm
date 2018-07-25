use llvm::Builder;
use llvm::Compile;
use llvm::Context as LLVMCtx;
use llvm::Module as LLVMModule;
use llvm::Sub;
use llvm::Value;

use wasmparser::Type;

use super::super::wasm::Global;
use super::super::wasm::Instruction;
use super::super::Opt;

use super::type_conversions::llvm_type_to_wasm_type;
use super::type_conversions::wasm_type_to_llvm_type;

pub enum GlobalValue<'a> {
    Native(&'a Value),
    InlinedConstant(&'a Value),
}

impl<'a> GlobalValue<'a> {
    pub fn load(&self, b: &'a Builder) -> &'a Value {
        match self {
            GlobalValue::Native(ptr) => b.build_load(ptr),
            GlobalValue::InlinedConstant(v) => v,
        }
    }

    pub fn store(&self, b: &'a Builder, v: &'a Value) {
        match self {
            GlobalValue::Native(ptr) => {
                b.build_store(v, ptr);
            }
            GlobalValue::InlinedConstant(_) => panic!("Cannot write to an inlined constant"),
        }
    }
}

pub fn insert_globals<'a>(
    opt: &Opt,
    llvm_ctx: &'a LLVMCtx,
    llvm_module: &'a LLVMModule,
    globals: Vec<Global>,
) -> Vec<GlobalValue<'a>> {
    let mut global_values = Vec::new();
    for (i, g) in globals.into_iter().enumerate() {
        let v = insert_native_global(opt, &*llvm_ctx, &*llvm_module, g);
        global_values.push(v);
    }
    global_values
}

fn insert_native_global<'a>(
    opt: &Opt,
    llvm_ctx: &'a LLVMCtx,
    llvm_module: &'a LLVMModule,
    g: Global,
) -> GlobalValue<'a> {
    match g {
        Global::Imported {
            name,
            content_type,
            mutable,
        } => {
            let llvm_global =
                llvm_module.add_global(&name, wasm_type_to_llvm_type(&*llvm_ctx, content_type));
            llvm_global.set_constant(!mutable);
            GlobalValue::Native(llvm_global.to_super())
        }
        Global::InModule {
            generated_name,
            content_type,
            mutable,
            initializer,
        } => {
            let v = initializer_to_value(llvm_ctx, content_type, initializer);
            if opt.inline_constant_globals && !mutable {
                GlobalValue::InlinedConstant(v)
            } else {
                let llvm_global = llvm_module.add_global_variable(&generated_name, v);
                llvm_global.set_constant(!mutable);
                GlobalValue::Native(llvm_global.to_super())
            }
        }
    }
}

fn initializer_to_value<'a>(
    llvm_ctx: &'a LLVMCtx,
    content_type: Type,
    initializer: Vec<Instruction>,
) -> &'a Value {
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

    v
}
