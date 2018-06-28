#![feature(nll)]

extern crate llvm;
extern crate wasmparser;

use std::fs::File;
use std::io;
use std::io::Read;
use std::path::Path;

use wasmparser::Parser;

mod codegen;
use codegen::process_to_llvm;

mod wasm;
use wasm::WasmModule;

fn main() -> io::Result<()> {
    let input_path_string = "input.wasm";
    let input_path: &Path = (&input_path_string).as_ref();
    let input_filename = input_path.file_name().and_then(|s| s.to_str()).unwrap();

    let mut wasm_file = File::open(input_path)?;

    let mut wasm_bytes = Vec::new();
    wasm_file.read_to_end(&mut wasm_bytes)?;

    let mut parser = Parser::new(&wasm_bytes);
    let module = WasmModule::from_wasm_parser(input_filename, &mut parser);

    let output_path = "output.bc";
    process_to_llvm(module, output_path);

    Ok(())
}
