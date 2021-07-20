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

AWSM_CC:=${ROOT_PATH}/target/release/awsm

# Use WASI_SDK if set to true. Wasmception otherwise
USE_WASI_SDK:=true

WASMCEPTION_PATH:=${ROOT_PATH}/wasmception
WASMCEPTION_CLANG:=${WASMCEPTION_PATH}/dist/bin/clang
WASMCEPTION_SYSROOT:=${WASMCEPTION_PATH}/sysroot
WASMCEPTION_FLAGS:=--target=wasm32-unknown-unknown-wasm -nostartfiles ${OPTFLAGS} --sysroot=${WASMCEPTION_SYSROOT}
WASMCEPTION_BACKING:=wasmception_backing.c

WASI_SDK_PATH:=${ROOT_PATH}/wasi-sdk
WASI_SDK_CLANG:=${WASI_SDK_PATH}/bin/clang
WASI_SDK_SYSROOT:=${WASI_SDK_PATH}/share/wasi-sysroot
WASI_SDK_FLAGS:=--target=wasm32-wasi -mcpu=mvp -nostartfiles ${OPTFLAGS} --sysroot=${WASI_SDK_SYSROOT}
WASI_SDK_BACKING:=wasi_sdk_backing.c

ifeq ($(USE_WASI_SDK), true)
	WASM_CC:=${WASI_SDK_CLANG}
	WASM_FLAGS:=${WASI_SDK_FLAGS}
	WASM_BACKING:=${WASI_SDK_BACKING}
else
	WASM_CC:=${WASMCEPTION_CLANG}
	WASM_FLAGS:=${WASMCEPTION_FLAGS}
	WASM_BACKING:=${WASMCEPTION_BACKING}
endif

# Excludes stack-size, as this may vary between apps
WASM_LINKER_FLAGS:=-Wl,--allow-undefined,--no-threads,--stack-first,--no-entry,--export-all,--export=main,--export=dummy

app_pid: app_pid_native app_pid_np_us app_pid_np app_pid_bc app_pid_vm

.PHONY: app_pid_clean
app_pid_clean:
	@rm -f ./app_pid/bin/*

clean: app_pid_clean

# TODO: Directly executing the native binary does not write output to the console as with the
# various awsm builds. This behavior is observable when building with run.py as well.
.PHONY: app_pid_native
app_pid_native:
	@mkdir -p ./app_pid/bin
	@${CC} -lm ${OPTFLAGS} -Wall -std=c++11 ./app_pid/*.cpp -o ./app_pid/bin/app_pid_native

.PHONY: app_pid.wasm
app_pid.wasm: ./dummy.cpp ./app_pid/*.cpp
ifeq ($(USE_WASI_SDK), true)
	@echo "Building with WASK-SDK"
else
	@echo "Building with Wasmception"
endif
	@mkdir -p ./app_pid/bin
	@${WASM_CC} ${WASM_FLAGS} ${WASM_LINKER_FLAGS} -Wl,-z,stack-size=256  -Wall -std=c++11 ./dummy.cpp ./app_pid/*.cpp -o ./app_pid/bin/app_pid.wasm

.PHONY: app_pid.bc
app_pid.bc: app_pid.wasm
	@mkdir -p ./app_pid/bin
	@${AWSM_CC} ./app_pid/bin/app_pid.wasm -o ./app_pid/bin/app_pid.bc
	
# Generate LLVM bitcode using the "fast unsafe implementations" option
.PHONY: app_pid_us.bc
app_pid_us.bc: app_pid.wasm
	@mkdir -p ./app_pid/bin
	@${AWSM_CC} -u ./app_pid/bin/app_pid.wasm -o ./app_pid/bin/app_pid_us.bc

# Run unsafe code using the "No Protection" memory model
.PHONY: app_pid_np_us
app_pid_np_us: app_pid_us.bc
	@${CC} -lm ${OPTFLAGS} \
	./app_pid/bin/app_pid_us.bc \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/${WASM_BACKING} \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/no_protection.c \
	-o ./app_pid/bin/app_pid_np_us

# Run safe code using the "No Protection" memory model
.PHONY: app_pid_np
app_pid_np: app_pid.bc
	@${CC} -lm ${OPTFLAGS} \
	./app_pid/bin/app_pid.bc \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/${WASM_BACKING} \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/no_protection.c \
	-o ./app_pid/bin/app_pid_np

# Run safe code using the "Generic" memory model
.PHONY: app_pid_bc
app_pid_bc: app_pid.bc
	@${CC} -lm ${OPTFLAGS} \
	./app_pid/bin/app_pid.bc \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/${WASM_BACKING} \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/generic.c \
	-o ./app_pid/bin/app_pid_bc

# Run safe code using the "Virtual Memory" memory model
.PHONY: app_pid_vm
app_pid_vm: app_pid.bc
	@${CC} -lm ${OPTFLAGS} \
	./app_pid/bin/app_pid.bc \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/${WASM_BACKING} \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/64bit_nix.c \
	-o ./app_pid/bin/app_pid_vm
