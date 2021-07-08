CC=clang
OPTFLAGS=-O3 -flto
UNAME:=$(shell uname -m)


# Assuming to be called from code_benches directory
ROOT_PATH:=$(shell cd ../.. && realpath .)
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

clean:
	rm -f if.wasm if.bc if_vm

if.wasm:
	wat2wasm if.wat -o if.wasm

# Should work for Linux or Max. Alternate installation options at https://deno.land/#installation
.PHONY: install_deno
install_deno:
	curl -fsSL https://deno.land/x/install/install.sh | sh

.PHONY: install_wabt
install_wabt:
	./install_wabt.sh

.PHONY: if_deno
if_deno: if.wasm
	deno run --allow-read if.ts

if.bc: if.wasm
	RUST_BACKTRACE=1 ${AWSM_CC} ./if.wasm -o ./if.bc

if_vm: if.bc
	@${CC} -lm ${OPTFLAGS} \
	./if.bc \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/${WASM_BACKING} \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/64bit_nix.c \
	-o ./if_vm