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

# Installation from Source

## Debian-based Systems
```sh
git clone https://github.com/gwsystems/silverfish.git
cd silverfish
./install_deb.sh
```

The compiler can now be run via `silverfish`

## Others
1. [Install Rust](https://www.rust-lang.org/tools/install)
2. [Install LLVM](http://releases.llvm.org/download.html)
3. Link Your Clang Installation so `clang-9` and `llvm-config-9` are in your path and invokable as `clang` and `llvm-config`. For example, the following commands accomplish this using `update-alternatives` after replacing LLVM_VERSION with the version returned you installed above (In *NIX systems, this can be confirmed with `ls /usr/bin/clang*`)
```sh
LLVM_VERSION=9
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-$LLVM_VERSION 100
sudo update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-$LLVM_VERSION 100
```
4. [Install the C++ Standard Library for LLVM](https://libcxx.llvm.org/)
​
5. Clone and build silverfish
```sh
git clone https://github.com/gwsystems/silverfish.git
cd silverfish
cargo build --release
```
6. The silverfish binary is built at `target/release/silverfish`. Copy this to the appropriaate place for your platform and add to your PATH if neccessary.
