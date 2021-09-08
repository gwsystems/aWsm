# Abstract WASI Interface

This directory contains an abstract interface that relieves some of the burdens of developing backing implementations of the WASI syscalls. The abstract interface performs bounds checking and serialization/deserialization of data (converting back and forth between WebAssembly Linear Memory offsets and native host pointers). A backing implementation that follows these patterns is passed native pointers with appropriate type casts. This closely resembles the WASI-SDK header file that a source C program can include to make WASI calls directly, but with an additional "context" handle prepended to each call.

Note: In the case of certain types, such as strings, a modified form of the WASI "Interface Type" is passed. Typically, this is a tuple containing a base linear memory offset to the start of the string and a length. Our abstract interface converts the linear memory into a char \* pointing to the appropriate base address. However, the string length is also provided. Most of our test programs are written in C and seem to retain a null-terminator at the end of their strings, but this behavior likely differs for WebAssembly compiled from Rust or other languages. As such, strings should not be assumed to be properly null terminated, and the provided string length should always be used.

This repo contains two sample implementations of the WASI abstract interface.

1. wasi_impl_uvwasi - This is an implementation that delegates to the libuv-based uvwasi implementation used by Node.js. It is full-featured and passes all of the WASI tests.
2. wasi_impl_minimal - This is an implementation written from scratch that assumes the presence of a a full-featured POSIX environment. It does not implement all system calls.

## Steps to create your own backing

1. Copy the wasi_impl_minimal implementation. It is a good starting point for implementing your own wasi backing.

2. Modify the Context Struct, wasi_context_init, and wasi_context_destroy functions as required

The abstract interface is specified in wasi_backing.h and wasi_backing.c and includes the definition of a wasi_options_t struct common to all backings. Each backing is responsible for implementing a wasi_context_init function that uses such a struct to allocate and initialize a context struct. This is returned to the caller as an opaque handle. The caller passes this handle to wasi_context_destroy to clean up the context when the sandboxed code has run to completion. The author of a WASI backing is responsible for deciding the structure and layout of the context object and how to interpret the provided options struct.

3. Replace the `__wasi_*` function bodies with logic appropriate to your environment. The `wasi_impl_minimal` assumes a full-featured POSIX environment, but a backing for a microcontroller or RTOS might look quite different. Depending on your requirements, you can implement a subset of WASI syscalls. LTO and the WebAssembly import semantics can ensure that unused symbols will not be in the resulting binary.
