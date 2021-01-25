extern crate llvm;
#[macro_use]
extern crate log;
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
use crate::codegen::process_to_llvm;

mod wasm;
use crate::wasm::WasmModule;

pub mod llvm_externs;

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
    #[structopt(short = "i", long = "inline-constant-globals")]
    inline_constant_globals: bool,

    /// Allow unsafe instruction implementations that may be faster
    #[structopt(short = "u", long = "fast-unsafe-implementations")]
    use_fast_unsafe_implementations: bool,

    /// Don't generate native globals, let the runtime handle it
    #[structopt(long = "runtime-globals")]
    use_runtime_global_handling: bool,

    /// Set compilation target
    #[structopt(long = "target")]
    target: Option<String>,

    /// Set compilation data layout
    #[structopt(long = "layout")]
    layout: Option<String>,
}

fn main() -> io::Result<()> {
    let log_spec = "debug";
    flexi_logger::Logger::with_str(log_spec).start().unwrap();

    let opt = Opt::from_args();
    info!("running silverfish({:?}, {:?})", opt.input, opt.output);

    let input_path: &Path = &opt.input;
    let input_filename = input_path.file_name().and_then(|s| s.to_str()).unwrap();

    let mut wasm_file = File::open(input_path)?;

    let mut wasm_bytes = Vec::new();
    wasm_file.read_to_end(&mut wasm_bytes)?;

    let mut parser = Parser::new(&wasm_bytes);
    let module = WasmModule::from_wasm_parser(input_filename, &mut parser);

    // Get diagnostics
    module.log_diagnostics();

    let output_path = opt
        .output
        .clone()
        .unwrap_or_else(|| "output.bc".to_string());
    process_to_llvm(&opt, module, &output_path)?;

    info!("silverfish finished successfully");
    Ok(())
}
