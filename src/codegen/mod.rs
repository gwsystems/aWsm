use std::io;

use llvm::Context as LLVMCtx;
use llvm::Function as LLVMFunction;
use llvm::Module as LLVMModule;

use wasmparser::FuncType;

use crate::wasm::Export;
use crate::wasm::Function;
use crate::wasm::WasmModule;

use super::Opt;

mod block;

mod breakout;

mod function;
use self::function::compile_function;

mod globals;
use self::globals::insert_globals;
use self::globals::GlobalValue;

mod memory;
use self::memory::add_memory_size_globals;
use self::memory::generate_memory_initialization_stub;

mod runtime_stubs;
use self::runtime_stubs::insert_runtime_stubs;

mod table;
use self::table::generate_table_initialization_stub;

mod type_conversions;
use self::type_conversions::wasm_func_type_to_llvm_type;

pub struct ModuleCtx<'a> {
    opt: &'a Opt,
    llvm_ctx: &'a LLVMCtx,
    llvm_module: &'a LLVMModule,
    types: &'a [FuncType],
    globals: &'a [GlobalValue<'a>],
    functions: &'a [(&'a LLVMFunction, Function)],
}

pub fn process_to_llvm(
    opt: &Opt,
    mut wasm_module: WasmModule,
    output_path: &str,
) -> io::Result<()> {
    let llvm_ctx = &*LLVMCtx::new();
    let llvm_module = &*LLVMModule::new(&wasm_module.source_name, llvm_ctx);

    // Accept --target to compile for specific target, otherwise omit target
    // triple from bytecode, this defaults to the host target in LLVM
    if let Some(ref target) = opt.target {
        llvm_module.set_target(target);
    }

    // Remap WASM generated names to exported names
    for e in wasm_module.exports {
        match e {
            Export::Function { index, name } => {
                // We namespace these, so they can't conflict with our actual native code
                wasm_module.functions[index].set_name("wasmf_".to_string() + &name);
            }
            Export::Global { index, name } => {
                // We namespace these, so they can't conflict with our actual native code
                wasm_module.globals[index].set_name("wasmg_".to_string() + &name);
            }
            // Exporting memory is meaningless in our native embedding
            Export::Memory { .. } => {}
        }
    }

    // We need to insert runtime stubs, because code generation will call them for certain instructions
    insert_runtime_stubs(opt, &*llvm_ctx, &*llvm_module);

    // Wasm globals have a natural mapping to llvm globals
    let globals = insert_globals(&opt, llvm_ctx, llvm_module, wasm_module.globals);

    // We need to prototype functions before implementing any, in case a function calls a function implemented after it
    let mut functions = Vec::new();
    for f in &wasm_module.functions {
        let llvm_f = llvm_module.add_function(
            f.get_name(),
            wasm_func_type_to_llvm_type(&llvm_ctx, f.get_type()),
        );
        functions.push((&*llvm_f, f.clone()));
    }

    // The global information about a module makes up the module context
    let module_ctx = ModuleCtx {
        opt,
        llvm_ctx,
        llvm_module,
        types: wasm_module.types.as_slice(),
        functions: functions.as_slice(),
        globals: globals.as_slice(),
    };

    // We assume there is only one relevent memory
    assert_eq!(wasm_module.memories.len(), 1);

    // The runtime needs to know how big the memory is/can be
    add_memory_size_globals(&module_ctx, &wasm_module.memories[0].limits);

    // Which we then need to initialize the data
    generate_memory_initialization_stub(&module_ctx, wasm_module.data_initializers);

    // Assume there is only one relevent table
    assert_eq!(wasm_module.tables.len(), 1);
    // TODO: Do some sort of dynamic handling of table size
    assert!(wasm_module.tables[0].limits.initial <= 1024);
    assert!(wasm_module.tables[0].limits.maximum.unwrap_or(0) <= 1024);

    generate_table_initialization_stub(&module_ctx, wasm_module.table_initializers);

    // Next we implement the implemented functions
    for f in wasm_module.functions {
        if let Function::Implemented { f } = f {
            compile_function(&module_ctx, &f);
        }
    }

    // TODO: Remove this debugging print
    //        llvm_module.dump();

    llvm_module.write_bitcode(output_path)
}
