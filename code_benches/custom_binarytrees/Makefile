CC=clang
OPTFLAGS=-O3 -flto
LLVM_VERSION=12

# Assuming to be called from code_benches/custom_binarytrees directory
ROOT_PATH:=$(shell cd ../.. && realpath .)
RUNTIME_PATH:=${ROOT_PATH}/runtime

AWSM_CC:=${ROOT_PATH}/target/debug/awsm

# Use WASI_SDK if set to true. Wasmception otherwise
USE_WASI_SDK:=false

WASMCEPTION_PATH:=${ROOT_PATH}/wasmception
WASMCEPTION_CLANG:=${WASMCEPTION_PATH}/dist/bin/clang
WASMCEPTION_SYSROOT:=${WASMCEPTION_PATH}/sysroot
WASMCEPTION_FLAGS:=--target=wasm32-unknown-unknown-wasm -nostartfiles ${OPTFLAGS} --sysroot=${WASMCEPTION_SYSROOT}
WASMCEPTION_BACKING:=wasmception_backing.c
WASMCEPTION_LINKER_FLAGS:=-Wl,--allow-undefined,--no-threads,--stack-first,--no-entry,--export-all,--export=main,--export=dummy
ifeq ($(WASI_SDK_PATH), "")
	WASI_SDK_PATH:=${ROOT_PATH}/wasi-sdk
endif

WASI_SDK_CLANG:=${WASI_SDK_PATH}/bin/clang
WASI_SDK_SYSROOT:=${WASI_SDK_PATH}/share/wasi-sysroot
WASI_SDK_FLAGS:=--target=wasm32-wasi -mcpu=mvp ${OPTFLAGS} --sysroot=${WASI_SDK_SYSROOT}
WASI_SDK_BACKING:=wasi_sdk_backing.c
WASI_SDK_LINKER_FLAGS:=-Wl,--allow-undefined,--stack-first,--threads=1

ifeq ($(USE_WASI_SDK), true)
	WASM_CC:=${WASI_SDK_CLANG}
	WASM_FLAGS:=${WASI_SDK_FLAGS}
	WASM_BACKING:=${WASI_SDK_BACKING}
	WASM_LINKER_FLAGS:=${WASI_SDK_LINKER_FLAGS}
else
	WASM_CC:=${WASMCEPTION_CLANG}
	WASM_FLAGS:=${WASMCEPTION_FLAGS}
	WASM_BACKING:=${WASMCEPTION_BACKING}
	WASM_LINKER_FLAGS:=${WASMCEPTION_LINKER_FLAGS}
endif

RUNTIME_NO_MEM:=${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/${WASM_BACKING} \
	${RUNTIME_PATH}/libc/env.c

RUNTIME_GENERIC:=${RUNTIME_NO_MEM} ${RUNTIME_PATH}/memory/generic.c
RUNTIME_NP:=${RUNTIME_NO_MEM} ${RUNTIME_PATH}/memory/no_protection.c
RUNTIME_VM:=${RUNTIME_NO_MEM} ${RUNTIME_PATH}/memory/64bit_nix.c

# Excludes stack-size, as this may vary between apps

.PHONY: all
all: bin/custom_binarytrees bin/custom_binarytrees.wasm bin/custom_binarytrees.bc bin/custom_binarytrees.ll bin/custom_binarytrees_us.bc bin/custom_binarytrees_us.ll bin/custom_binarytrees_np_us bin/custom_binarytrees_np bin/custom_binarytrees_bc bin/custom_binarytrees_vm

.PHONY: clean
clean:
	@rm -rf bin

bin/custom_binarytrees: *.c
	@mkdir -p bin
	@${CC} -lm ${OPTFLAGS} -I. -Wall *.c -o bin/custom_binarytrees

bin/custom_binarytrees.wasm: ../dummy.c *.c
ifeq ($(USE_WASI_SDK), true)
	@echo "Building with WASK-SDK"
else
	@echo "Building with Wasmception"
endif
	@mkdir -p bin
	${WASM_CC} ${WASM_FLAGS} ${WASM_LINKER_FLAGS} -I. -Wall ../dummy.c *.c -o ./bin/custom_binarytrees.wasm

# Generate LLVM bitcode with "fast unsafe implementations" disabled
bin/custom_binarytrees.bc: bin/custom_binarytrees.wasm
	@${AWSM_CC} $^ -o $@
	
# Generate LLVM bitcode with "fast unsafe implementations" enabled
bin/custom_binarytrees_us.bc: bin/custom_binarytrees.wasm
	@${AWSM_CC} -u $^ -o $@

%.ll: %.bc
	llvm-dis-${LLVM_VERSION} $^ -o $@

# Run unsafe code using the "No Protection" memory model
bin/custom_binarytrees_np_us: bin/custom_binarytrees_us.bc ${RUNTIME_NP}
	@${CC} -lm ${OPTFLAGS} $^ -o $@

# Run safe code using the "No Protection" memory model
bin/custom_binarytrees_np: bin/custom_binarytrees.bc ${RUNTIME_NP}
	@${CC} -lm ${OPTFLAGS} $^ -o $@

# Run safe code using the "Generic" memory model
bin/custom_binarytrees_bc: bin/custom_binarytrees.bc ${RUNTIME_GENERIC}
	@${CC} -lm ${OPTFLAGS} $^ -o $@

# Run safe code using the "Virtual Memory" memory model
bin/custom_binarytrees_vm: bin/custom_binarytrees.bc ${RUNTIME_VM}
	@${CC} -lm ${OPTFLAGS} $^ -o $@
