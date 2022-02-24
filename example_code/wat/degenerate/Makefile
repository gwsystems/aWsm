# Sample awsm Makefile
#
# This project currently uses a hand-spun build system and test running written (run.py)
# The intent behind this sample makefile is to illustrate how one might use awsm using
# a standard build tool.

CC=clang
OPTFLAGS=-O3 -flto
UNAME:=$(shell uname -m)

# Assuming to be called from code_benches directory
ROOT_PATH:=$(shell cd ../../.. && realpath .)
RUNTIME_PATH:=${ROOT_PATH}/runtime

RUNTIME_CPATH+=${RUNTIME_PATH}/runtime.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_main.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_backing.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_impl_uvwasi.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/env.c
RUNTIME_CPATH+=${RUNTIME_PATH}/memory/64bit_nix.c
RUNTIME_CPATH+=${RUNTIME_PATH}/thirdparty/dist/lib/libuv_a.a
RUNTIME_CPATH+=${RUNTIME_PATH}/thirdparty/dist/lib/libuvwasi_a.a

RUNTIME_INCLUDES=-I${RUNTIME_PATH}/libc/wasi/include -I${RUNTIME_PATH}/thirdparty/dist/include

AWSM_CC:=${ROOT_PATH}/target/release/awsm

# Excludes stack-size, as this may vary between apps
WASM_LINKER_FLAGS:=-Wl,-z,stack-size=524288,--allow-undefined,--threads=1

clean:
	rm -f degenerate.wasm
	rm -f degenerate.bc
	rm -f degenerate.ll
	rm -f degenerate_vm

degenerate.wasm: degenerate.wat
	wat2wasm degenerate.wat

degenerate.bc: degenerate.wasm
	${AWSM_CC} $< -o $@

degenerate.ll: degenerate.bc
	llvm-dis $< > $@

degenerate_vm: degenerate.bc
	${CC} -lm -lpthread -ldl ${OPTFLAGS} ${RUNTIME_INCLUDES} ${RUNTIME_CPATH} $< -o $@
