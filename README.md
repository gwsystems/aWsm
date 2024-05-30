aWsm - An Awesome Wasm Compiler and Runtime
==========
_Note: Previously known as Silverfish_

# What is aWsm?

aWsm is a compiler and runtime for compiling WebAssembly (Wasm) code into llvm bytecode, then into sandboxed binaries you can run on various platforms.
It focuses on generating very fast code (best of breed for WebAssembly), having a simple and extensible code-base, and on portability.

**What is WebAssembly?**
Wasm is a standard binary format that is *platform agnostic*, *debuggable*, *fast*, and *safe*.
Please see the [official webpage](https://webassembly.org/), and its [specification](https://webassembly.org/specs/).
Many languages (including C/C++, Rust, C#, Go, Kotlin, Swift, and more, see a more [complete list](https://webassembly.org/getting-started/developers-guide/)) compile to Wasm.
Wasm can be run in all the major browsers, thus is broadly deployed and available.

We're interested in taking Wasm out of the browser, and into all different types of computers (from servers to microcontrollers).
Wasm provides a number of benefits outside of the web including:

- *Sandboxing.*
	Regardless which language is compiled to Wasm, the Wasm runtime can execute them *safely* (including those languages with [footguns](https://en.wiktionary.org/wiki/footgun) like C).
	This uses Software Fault Isolation (SFI) -- to ensure that loads and stores are only within the sandbox-- and Control-Flow Integrity (CFI) -- only allowing the execution of code intentionally generated by the compiler -- to provide safe execution.
	Restricting SFI protects the surrounding code from faulty or malicious code, and CFI prevents a compromise from hijacking the flow of execution (see [buffer overflows](https://en.wikipedia.org/wiki/Buffer_overflow) and [ROP chains](https://en.wikipedia.org/wiki/Return-oriented_programming)).

	Sandboxing can act as a process replacement, especially on systems that don't have hardware support for processes, or where high-density is required.
- *Ubiquity.*
	Wasm is a high-level assembly, independent from any specific hardware.
	This has the potential to provide a universal specification of computation that can execute in the cloud, on the edge, or in an embedded device.

## Why aWsm?

We've investigated how aWsm changes serverless on the edge (see our [Sledge runtime](https://github.com/gwsystems/sledge-serverless-framework) for that), and reliability in embedded systems.
See the publications and videos:

- [*Sledge: a Serverless-first, Light-weight Wasm Runtime for the Edge*](https://www2.seas.gwu.edu/~gparmer/publications/middleware20sledge.pdf) at ACM Middleware, 2020 ([video](https://www.youtube.com/watch?v=dxsYAzVMxH8))
- [*eWASM: Practical Software Fault Isolation for Reliable Embedded Devices*](https://www2.seas.gwu.edu/~gparmer/publications/emsoft20wasm.pdf) at EMSOFT, 2020 ([video](https://www.youtube.com/watch?v=x5j4AuwZbYE))

Why does aWsm enable these cool domains?
The Web Assembly eco-system is still developing, and we see the need for a system focusing on:

- *Performance.*
	aWsm is an ahead-of-time compiler that leverages the LLVM compiler to optimize code, and target different architectural backends.
	We have evaluated aWsm on x86-64, aarch64 (Raspberry Pi), and thumb (ARM Cortex-M4 and M7), and performance on the  microprocessors is within 10% of native, and within 40% on the microcontrollers on Polybench benchmarks.
- *Simplicity.*
	The entire code base for the compiler and runtime is relatively small.
	The compiler is <3.5K lines of Rust, and the runtime (for *all* platforms) is <5K lines of C.
	It is nearly trivial to implement different means of sandboxing memory accesses.
	We've implemented *seven* different mechanisms for this!
- *Portability.*
	Both the compiler and runtime are mostly platform-independent code, and porting to a new platform only really requires additional work if you need to tweak stack sizes (microcontrollers), or use architectural features (e.g., MPX, segmentation, etc...).
	aWsm only links what is needed, so it's possible to avoid microcontroller-expensive operations such as f64, f32, and even dynamic memory.
- *Composability.*
	The final output of aWsm is simple `*.o` elf objects that can be linked into larger systems.
	This enables the trivial composition of sandboxes together, and sandboxes into larger programs.

We believe that aWsm is one of the best options for ahead-of-time compilation for Wasm execution outside of the browser.
If you want to learn more about aWsm, see the [design](doc/design.md), or the publications listed above.

## Where does aWsm Help Out?

- **Edge** and **Serverless**:
	aWsm enables small-footprint, efficient edge, serverless runtime that we call [sledge](https://github.com/gwsystems/sledge-serverless-framework/blob/master/README.md) (or ServerLess on the EDGE).
	aWsm sandboxes don't take up many more resources than a native binary, and sledge can create sandboxes, thus edge functions, in around 50 microseconds.
	This enables massive benefits on the latency-sensitive edge!
- **Embedded devices**:
	aWsm is used in [eWasm](https://www2.seas.gwu.edu/~gparmer/publications/emsoft20wasm.pdf) to provide sandboxed isolation in small microcontrollers (~64-128KiB of SRAM!).
	aWsm enables memory savings compared to other embedded techniques, and near-native C performance.
	aWsm is used in [bento boxes](https://github.com/arm-software/bento-linker) that provide an abstraction for embedded processes.
	They also have shown the huge [performance benefit of aWsm](https://github.com/arm-software/bento-linker#results).
- **Cloud**:
	Similar to the serverless argument above, aWsm can provide additional security by sandboxing tenant computation.
	aWsm makes it trivial to filter system calls and system requires in accordance with your security policies.
- **Desktop**:
	Similarly, aWsm can provide increased security where necessary.
- **Embedding policy into frameworks**:
	Lua is often used to embed policy into C++ game engines, and eBPF is used to embed logic into the Linux kernel.
	Wasm can be targeted by many languages, making it an appealing target, and aWsm enables it to be efficiently embedded into the surrounding framework.

Where aWsm isn't *yet* a great fit:

- Browsers:
	Browsers have their own Wasm engines that can't easily be replaced.
	Until the browser makers are breaking down our door to get aWsm in their browser, you'll have to use their built-in engines.
- Wherever interpreters or Jit are required:
	Some domains focus almost entirely on the delay from download to execution.
	An AoT compiler is not a great fit for that domain due to the latency of compilation.
	However, aWsm has been demonstrated to generate code that has exceptional performance, so we focus on domains that benefit from efficiency.

# Performance

PolyBench/C benchmarks for **x86-64** (slowdown over native C):

| | Wasmer | WAVM | Node.js + Emscripten | Lucet | aWsm |
| --- | --- | --- | --- | --- | --- |
| Avg. Slowdown | 149.8% | 28.1% | 84.0% | 92.8% | 13.4% |
| Stdev. in Slowdown | 194.09 | 53.09 | 107.84 | 117.25 | 34.65 |

PolyBench/C benchmarks for **Arm aarch64** (slowdown over native C):

| | aWsm |
| --- | --- |
| Avg. Slowdown | 6.7% |
| Stdev. of Slowdown | 19.38 |

Polybench/C benchmarks for **Arm Cortex-M** microcontrollers (slowdown over native C):

| Architectures | aWsm |
| --- | --- |
| Cortex-M7 Avg. slowdown | 40.2% |
| Cortex-M4 Avg. slowdown | 24.9% |

In comparison, the [`wasm3` interpreter's slowdown](https://github.com/wasm3/wasm3/blob/master/docs/Performance.md) on microcontrollers is more than 10x.
For more details (including other bounds checking mechanisms), see the [paper](https://www2.seas.gwu.edu/~gparmer/publications/emsoft20wasm.pdf).

*Note: these numbers are from May 2020.*

There are many compelling runtimes, but we believe that aWsm is useful in generating very fast code, while being simple and extensible.

## Comparison to Existing Wasm Ecosystems

There are many existing compilers and runtimes.
aWsm fills the niche of a compiler

- based on ahead-of-time compilation using the popular LLVM infrastructure,
- that generates fast, safe code (even on microcontrollers), and
- that is designed for to be lightweight and easily extended.

Adding runtime functions and changing safety checking mechanisms are trivial operations.

| Runtime |	Method | x86_64 | x86 | aarch64 | thumb | URL |
| --- | --- | --- | --- | --- | --- | --- |
| aWsm | 	AoT | 	✓ | 	✓ | 	✓ | 	✓ | You are here |
| Wasmtime | 	AoT | 	✓ | 	✓ | 	✓ | 	| https://github.com/bytecodealliance/wasmtime |
| Wasmer |  	AoT |  	✓ |  	✓ |  	✓ |  	|	https://github.com/wasmerio/wasmer |
| WAMR |  	Pseudo-AoT/Int |  	✓ |  	✓ |  	✓ |  	✓ |  	https://github.com/bytecodealliance/wasm-micro-runtime |
| Wasm3 |  	Int |  	✓ |  	✓ |  	✓ |  	✓ |  	https://github.com/wasm3/wasm3 |
| Wasmi |  	Int |  	✓ |  	✓ |  	✓ |  	✓ |  	https://github.com/paritytech/wasmi |

This is not an exhaustive list!
There are many others as this is a pretty active area.

*Note: this table is from the best of our understanding of each system in July 2020.*

# Getting started!

```
aWsm 0.1.0
Gregor Peach <gregorpeach@gmail.com>

USAGE:
    awsm [FLAGS] [OPTIONS] <input>

FLAGS:
    -h, --help                           Prints help information
    -i, --inline-constant-globals        Force inlining of constant globals
    -u, --fast-unsafe-implementations    Allow unsafe instruction implementations that may be faster
        --runtime-globals                Don't generate native globals, let the runtime handle it
    -V, --version                        Prints version information

OPTIONS:
    -o, --output <output>    Output bc file
        --target <target>    Set compilation target

ARGS:
    <input>    Input wasm file
```

## Installation from Source

### Debian-based Systems

```sh
git clone https://github.com/gwsystems/aWsm.git
cd aWsm
./install_deb.sh
```

The compiler can now be run via `awsm`

### Other Systems

1. [Install Rust](https://www.rust-lang.org/tools/install)
2. [Install LLVM](http://releases.llvm.org/download.html)
3. Link Your Clang Installation so `clang-xx` and `llvm-config-xx` are in your path aliased to `clang` and `llvm-config`. For example, the following commands accomplish this using `update-alternatives` after replacing LLVM_VERSION with the version returned you installed above (In *NIX systems, this can be confirmed with `ls /usr/bin/clang*`)
```sh
LLVM_VERSION=13
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-$LLVM_VERSION 100
sudo update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-$LLVM_VERSION 100
```
4. [Install the C++ Standard Library for LLVM](https://libcxx.llvm.org/)
5. Clone and build aWsm
```sh
git clone https://github.com/gwsystems/aWsm.git
cd aWsm
cargo build --release
```
6. The aWsm binary is built at `target/release/awsm`. Copy this to the appropriate place for your platform and add to your PATH if necessary.

## Executing and Testing aWsm

The tests can run with

```sh
cd code_benches; ./run.py
```

Please see the [design](doc/design.md) to understand the pipeline, and see `run.py` for an example of how to connect existing compilers (to generate Wasm, and generate machine code from LLVM IR) with aWsm.

Note that aWsm is still a research prototype, so might have some rough edges.
It is not currently turn-key.
You likely need to understand how to generate Wasm, and use an LLVM compile chain.
We're happy to accept PRs with fixes, and "quality of life" improvements.

We're in the processes of standing up a CI infrastructure.

## Tour of the Source

The source is organized as such:

- `src/` - the Rust source of the compiler, and the `awsm` binary.
	Look here for the logic for taking in Wasm, and generating LLVM bytecode.
	`cargo` guides the compilation of this code.
- `code_benches/` - This is a relatively large set of benchmarks, many of which are derived from Polybench (`pb_*`), MiBench (`mb_*`), or various applications (`custom_*` including NN inference, sqlite, a PID controller, and an extended Kalman filter).
	The `run.py` file guides the compilation and execution of these as effectively a set of unit tests, and is an example of the compilation structure of an application with the runtime in aWsm.
- `example_code/` - More atomic tests for specific Wasm features (bounds checks, indirect function invocations, etc...).
	This ensures that changes to the compiler don't break Wasm sandboxing, and provides regression tests.
- `runtime/` - The aWsm runtime.
	This includes most of the code that provides the sandboxing guarantees (for example, including bounds checks, indirect function call  type checks and indirection).
	Microcontroller-specific runtime code (for Arm Cortex-M processors) referred to in [`eWasm`](https://www2.seas.gwu.edu/~gparmer/publications/emsoft20wasm.pdf), is in `cortex_m_glue/` and `libc/cortex_m_backing.c`.
	Various pluggable bounds checks can be found in `runtime/memory/`.
	The runtime is compiled separately, and combined with the LLVM IR output by aWsm (using LTO) to generate the final sandboxed object.
- `doc/` - Documentation directory.
	See the `*.md` files there.

# Limitations and Assumptions

*Additional Wasm instruction support needed.*
aWsm has not focused on a complete implementation of every Wasm instruction, instead focusing on supporting the code generated by the LLVM front-end.
The code is extensible, and we've added instructions as needs-be.
Please make a PR with support for additional instructions if you need them!

*Limited support for future Wasm features.*
The features on the [Wasm roadmap](https://webassembly.org/roadmap/) are not supported.
We're quite interested in supporting as many of these feature sets as possible, and would happily accept PRs.

# Advice for Wasm Standardization

We've done quite a bit of work targeting microcontrollers (Arm Cortex-Ms), and have discovered a number of limitations of the current Wasm Specification for that domain.
We provide details in Section 7 of our [EMSOFT publication](https://www2.seas.gwu.edu/~gparmer/publications/emsoft20wasm.pdf).
We believe that some changes to the specification, or the creation of an embedded profile might be warranted.
The main limitations:

1. *Variant page sizes selected by the runtime.*
	Wasm uses 64KiB pages.
	That is far too large for embedded systems.
	aWsm uses smaller pages, while simulating the larger default pages.
	Enabling smaller pages would make Wasm's use on microcontrollers -- often with only 64KiB of memory total -- easier.
	In aWsm, the page size is configurable.
2. *Separation of read-only and read-write memory.*
	In Wasm, all data is accessed in linear memory, which provides strong sandboxing, but does not discriminate between RW and RO data.
	Microcontrollers often use [Execute in Place](https://en.wikipedia.org/wiki/Execute_in_place) (XIP) read-only memory, which allows RO data to be accessed and executed *directly from flash*.
	This is beneficial in microcontrollers with on the order of 64KiB of memory (SRAM), but around 1MIB of flash.
	Wasm does not separate RO and RW memory, preventing this optimization that is essential for density on such systems.
3. *Allow undefined behavior on Out of Bounds (OoB) accesses.*
	The specification requires any access outside of the allocated bounds of linear memory to be caught, and the sandbox terminated.
	We show in the publication that relaxing this requirement, and allowing undefined behavior on OoB accesses can significantly speed up execution, and shrink code sizes, while maintaining strong sandboxed isolation.

# Acknowledgements

aWsm was designed and implemented by Gregor Peach (@Others) while an undergraduate researcher in the GW Systems Lab. Demonstrating the value of the WebAssembly specification in a systems context, aWsm inspired the GW Systems Lab to fund and pursue follow-up WebAssembly-based research systems outside of its traditional focus on component-based operating systems.

Collaboration with members of the Arm Research team have been particularly instrumental in growing aWsm from its initial prototype. Additionally, financial support from SRC, Arm, and NSF have supported researchers working on aWsm and follow-up projects.

# About Us

The GWU Systems group focuses on low-level system design and implementation.
One of our main research vehicles is the [Composite](http://composite.seas.gwu.edu) component-based operating system, which we aim to integrate with Wasm through aWsm.
If you're interested in low-level hacking and system building, in secure and reliable systems, in embedded systems, or in models for parallelism, don't hesitate to contact [Gabe](http://www.seas.gwu.edu/~gparmer) about doing a PhD or becoming involved with the group.
