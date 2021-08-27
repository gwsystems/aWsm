# Sample awsm Makefile
#
# This project currently uses a hand-spun build system and test running written (run.py)
# The intent behind this sample makefile is to illustrate how one might use awsm using
# a standard build tool.

CC=clang
OPTFLAGS=-O3 -flto
UNAME:=$(shell uname -m)

# Assuming to be called from code_benches directory
ROOT_PATH:=$(shell cd .. && realpath .)
RUNTIME_PATH:=${ROOT_PATH}/runtime

RUNTIME_CPATH+=${RUNTIME_PATH}/runtime.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_main.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_backing.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_impl_uvwasi.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/env.c
RUNTIME_CPATH+=${RUNTIME_PATH}/memory/64bit_nix.c
RUNTIME_CPATH+=${RUNTIME_PATH}/thirdparty/dist/lib/libuv_a.a
RUNTIME_CPATH+=${RUNTIME_PATH}/thirdparty/dist/lib/libuvwasi_a.a

RUNTIME_INCLUDES=-I../runtime/libc/wasi/include -I../runtime/thirdparty/dist/include

WASMCC=/opt/wasi-sdk/bin/clang

AWSM_CC:=${ROOT_PATH}/target/release/awsm

# Excludes stack-size, as this may vary between apps
WASM_LINKER_FLAGS:=-Wl,-z,stack-size=524288,--allow-undefined,--threads=1

app_pid: app_pid_native app_pid_np_us app_pid_np app_pid_bc app_pid_vm

.PHONY: app_pid_clean
app_pid_clean:
	@rm -f ./app_pid/bin/*

clean: app_pid_clean

# TODO: Directly executing the native binary does not write output to the console as with the
# various awsm builds. This behavior is observable when building with run.py as well.
./app_pid/bin/app_pid_native:
	@mkdir -p ./app_pid/bin
	@${CC} -lm ${OPTFLAGS} -Wall -std=c++11 ./app_pid/*.cpp -o ./app_pid/bin/app_pid_native

./app_pid/bin/app_pid.wasm: ./app_pid/*.cpp
	@mkdir -p ./app_pid/bin
	echo "Building with WASK-SDK"
	mkdir -p ./app_pid/bin
	${WASMCC} ${OPTFLAGS} ${WASM_LINKER_FLAGS} -Wl,-z,stack-size=256  -Wall -std=c++11 ./app_pid/*.cpp -o ./app_pid/bin/app_pid.wasm

./app_pid/bin/app_pid.bc: ./app_pid/bin/app_pid.wasm
	mkdir -p ./app_pid/bin
	${AWSM_CC} ./app_pid/bin/app_pid.wasm -o ./app_pid/bin/app_pid.bc

./app_pid/bin/app_pid_vm: ./app_pid/bin/app_pid.bc
	@mkdir -p ./app_pid/bin
	${CC} -lm -lpthread -ldl ${OPTFLAGS} ${RUNTIME_INCLUDES} ${RUNTIME_CPATH} ./app_pid/bin/app_pid.bc -o ./app_pid/bin/app_pid_vm
