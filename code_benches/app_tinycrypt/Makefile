CC=clang
OPTFLAGS=-O3 -flto
UNAME:=$(shell uname -m)

# Assuming to be called from code_benches/app_tinycrypt directory
ROOT_PATH:=$(shell cd ../.. && realpath .)
RUNTIME_PATH:=${ROOT_PATH}/runtime

AWSM_CC:=${ROOT_PATH}/target/debug/awsm
# AWSM_CC:=${ROOT_PATH}/target/release/awsm

# Use WASI_SDK if set to true. Wasmception otherwise
USE_WASI_SDK:=true

WASMCEPTION_PATH:=${ROOT_PATH}/wasmception
WASMCEPTION_CLANG:=${WASMCEPTION_PATH}/dist/bin/clang
WASMCEPTION_SYSROOT:=${WASMCEPTION_PATH}/sysroot
WASMCEPTION_FLAGS:=--target=wasm32-unknown-unknown-wasm -nostartfiles ${OPTFLAGS} --sysroot=${WASMCEPTION_SYSROOT}
WASMCEPTION_BACKING:=wasmception_backing.c
WASMCEPTION_LINKER_FLAGS:=-Wl,--allow-undefined,--no-threads,--stack-first,--no-entry,--export-all,--export=main,--export=dummy

WASI_SDK_PATH:=${ROOT_PATH}/wasi-sdk
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

# Excludes stack-size, as this may vary between apps

app_pid: app_pid_native app_pid_np_us app_pid_np app_pid_bc app_pid_vm

.PHONY: clean
clean:
	@rm -rf bin

bin/app_tinycrypt: *.c *.h
	@mkdir -p bin
	@${CC} -lm ${OPTFLAGS} -I. -Wall *.c -o bin/app_tinycrypt

bin/app_tinycrypt.wasm: ../dummy.c *.c *.h
ifeq ($(USE_WASI_SDK), true)
	@echo "Building with WASK-SDK"
else
	@echo "Building with Wasmception"
endif
	@mkdir -p bin
	${WASM_CC} ${WASM_FLAGS} ${WASM_LINKER_FLAGS} -O2 -Wl,-z,stack-size=256 -I. -Wall ../dummy.c *.c -o ./bin/app_tinycrypt.wasm

bin/app_tinycrypt.bc: bin/app_tinycrypt.wasm
	@mkdir -p bin
	@${AWSM_CC} $^ -o $@ 2>$@.log
	
# Generate LLVM bitcode using the "fast unsafe implementations" option
bin/app_tinycrypt_us.bc: bin/app_tinycrypt.wasm
	@mkdir -p bin
	@${AWSM_CC} -u $^ -o $@

%.ll: %.bc
	llvm-dis-12 $^ -o $@

# Run unsafe code using the "No Protection" memory model
bin/app_pid_np_us: bin/app_pid_us.bc
	@${CC} -lm ${OPTFLAGS} \
	$^ \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/${WASM_BACKING} \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/no_protection.c \
	-o $@

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
