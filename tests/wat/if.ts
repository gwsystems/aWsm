// Minimal glue code requires to instantiate and manually run our WebAssembly module

const wasmCode = await Deno.readFile("./if.wasm");
const wasmModule = new WebAssembly.Module(wasmCode);
const wasmInstance = new WebAssembly.Instance(wasmModule);
const main = wasmInstance.exports.main as CallableFunction;
console.log(main().toString());
