Silverfish
==========

A platform for compiling WASM code into llvm bytecode

```
silverfish 0.1.0
Gregor Peach <gregorpeach@gmail.com>

USAGE:
    silverfish [FLAGS] [OPTIONS] <input>

FLAGS:
    -h, --help                           Prints help information
    -i, --inline-constant-globals        Force inlining of constant globals
    -u, --fast-unsafe-implementations    Allow unsafe instruction implementations that may be faster
        --runtime-globals                Don't generate native globals, let the runtime handle it
    -V, --version                        Prints version information

OPTIONS:
    -o, --output <output>    Output bc file

ARGS:
    <input>    Input wasm file

```
Relies on you having a sensible llvm toolchain on your computer (so we can call llvm-config)