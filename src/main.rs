#![feature(nll)]

extern crate llvm;
extern crate structopt;
extern crate wasmparser;

use std::fs::File;
use std::io;
use std::io::Read;
use std::path::Path;
use std::path::PathBuf;

use structopt::StructOpt;

use wasmparser::Parser;

mod codegen;
use codegen::process_to_llvm;

mod wasm;
use wasm::WasmModule;

#[derive(StructOpt, Debug)]
#[structopt(name = "silverfish")]
pub struct Opt {
    /// Input wasm file
    #[structopt(name = "input", parse(from_os_str))]
    input: PathBuf,

    /// Output bc file
    #[structopt(short = "o", long = "output")]
    output: Option<String>,

    /// Force inlining of constant globals
    #[structopt(short = "i", long = "inline_constant_globals")]
    inline_constant_globals: bool,

    /// Don't generate native globals, let the runtime handle it
    #[structopt(long = "runtime_globals")]
    use_runtime_global_handling: bool,
}

fn main() -> io::Result<()> {
    let opt = Opt::from_args();

    let input_path: &Path = &opt.input;
    let input_filename = input_path.file_name().and_then(|s| s.to_str()).unwrap();

    let mut wasm_file = File::open(input_path)?;

    let mut wasm_bytes = Vec::new();
    wasm_file.read_to_end(&mut wasm_bytes)?;

    let mut parser = Parser::new(&wasm_bytes);
    let module = WasmModule::from_wasm_parser(input_filename, &mut parser);

    let output_path = opt
        .output
        .clone()
        .unwrap_or_else(|| "output.bc".to_string());
    process_to_llvm(&opt, module, &output_path)?;

    Ok(())
}
