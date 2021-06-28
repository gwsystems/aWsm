CC=clang
WASMCC=wasm32-unknown-unknown-wasm-clang
OPTFLAGS=-O3 -flto
UNAME:=$(shell uname -m)
SUPPORTS_MPX:=$(shell grep -q mpx /proc/cpuinfo && echo "true")

# Assuming to be called from code_benches directory
ROOT_PATH:=$(shell cd .. && realpath .)
RUNTIME_PATH:=${ROOT_PATH}/runtime
WASMCEPTION_PATH:=${ROOT_PATH}/wasmception
WASMCEPTION_CLANG:=${WASMCEPTION_PATH}/dist/bin/clang
WASMCEPTION_SYSROOT:=${WASMCEPTION_PATH}/sysroot
WASMCEPTION_FLAGS:=--target=wasm32-unknown-unknown-wasm -nostartfiles ${OPTFLAGS} --sysroot=${WASMCEPTION_SYSROOT}
WASMCEPTION_BACKING:=wasmception_backing.c

# Excludes stack-size
WASM_LINKER_FLAGS:=-Wl,--allow-undefined,--no-threads,--stack-first,--no-entry,--export-all,--export=main,--export=dummy

.PHONY: test
test:
	echo ${WASMCEPTION_PATH}

app_pid: app_pid_native app_pid_np_us app_pid_np app_pid_bc app_pid_vm

.PHONY: app_pid_clean
app_pid_clean:
	rm ./app_pid/bin/*

# TODO: Directly executing the native binary does not write output to the console as with the
# various awsm builds. This behavior is observable when building with run.py as well.
.PHONY: app_pid_native
app_pid_native:
	mkdir -p ./app_pid/bin
	${CC} -lm ${OPTFLAGS} -Wall -std=c++11 ./app_pid/*.cpp -o ./app_pid/bin/app_pid_native

# Wasmception Example
.PHONY: app_pid.wasm
app_pid.wasm:
	mkdir -p ./app_pid/bin
	${WASMCEPTION_CLANG} ${WASMCEPTION_FLAGS} ${WASM_LINKER_FLAGS} -Wl,-z,stack-size=256  -Wall -std=c++11 ./dummy.cpp ./app_pid/*.cpp -o ./app_pid/bin/app_pid.wasm

# Assumes that awsm is in PATH
.PHONY: app_pid.bc
app_pid.bc: app_pid.wasm
	mkdir -p ./app_pid/bin
	awsm ./app_pid/bin/app_pid.wasm -o ./app_pid/bin/app_pid.bc
	
# Generate bitcode using the "fast unsafe implementations" option
.PHONY: app_pid_us.bc
app_pid_us.bc: app_pid.wasm
	mkdir -p ./app_pid/bin
	awsm -u ./app_pid/bin/app_pid.wasm -o ./app_pid/bin/app_pid_us.bc

# Run unsafe code using the "No Protection" memory model
.PHONY: app_pid_np_us
app_pid_np_us: app_pid_us.bc
	${CC} -lm ${OPTFLAGS} \
	./app_pid/bin/app_pid_us.bc \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/wasmception_backing.c \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/no_protection.c \
	-o ./app_pid/bin/app_pid_np_us

# Run safe code using the "No Protection" memory model
.PHONY: app_pid_np
app_pid_np: app_pid.bc
	${CC} -lm ${OPTFLAGS} \
	./app_pid/bin/app_pid.bc \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/wasmception_backing.c \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/no_protection.c \
	-o ./app_pid/bin/app_pid_np

# Run safe code using the "Generic" memory model
.PHONY: app_pid_bc
app_pid_bc: app_pid.bc
	${CC} -lm ${OPTFLAGS} \
	./app_pid/bin/app_pid.bc \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/wasmception_backing.c \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/generic.c \
	-o ./app_pid/bin/app_pid_bc

# Run safe code using the "Virtual Memory" memory model
.PHONY: app_pid_vm
app_pid_vm: app_pid.bc
	${CC} -lm ${OPTFLAGS} \
	./app_pid/bin/app_pid.bc \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/wasmception_backing.c \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/64bit_nix.c \
	-o ./app_pid/bin/app_pid_vm

# TODO: Untested. Confirm that "mpx" is actually listed under flags on supported CPUs
.PHONY: app_pid_mpx
app_pid_mpx: app_pid.bc
ifeq ($(SUPPORTS_MPX), true)
	${CC} -lm ${OPTFLAGS} \
	./app_pid/bin/app_pid.bc \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/wasmception_backing.c \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/mpx.c \
	-o ./app_pid/bin/app_pid_mpx
else
	@printf "Did not detect MPX flag\n"
endif

# TODO: Untested. Validate use of segmentation on 32-bit Intel.
# TODO: Optionally use cross-compilation
.PHONY: app_pid_sm
app_pid_sm: app_pid.bc
ifeq ($(UNAME), i686)
	${CC} -lm ${OPTFLAGS} \
	./app_pid/bin/app_pid.bc \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/wasmception_backing.c \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/segmented.c \
	-o ./app_pid/bin/app_pid_sm
else
	@printf "Saw %s, not i686\n" $(UNAME)
endif

# TODO: WASI-SDK Alternatives
