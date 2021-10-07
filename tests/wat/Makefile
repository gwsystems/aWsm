ROOT_PATH:=$(shell cd ../.. && realpath .)
RUNTIME_PATH:=${ROOT_PATH}/runtime

CC=clang
OPTFLAGS=-O3 -flto
AWSM_CC:=${ROOT_PATH}/target/debug/awsm

RUNTIME_CFILES+=${RUNTIME_PATH}/runtime.c
RUNTIME_CFILES+=${RUNTIME_PATH}/libc/env.c
RUNTIME_CFILES+=${RUNTIME_PATH}/memory/64bit_nix.c

WASMCEPTION_RUNTIME_CFILES=${RUNTIME_CFILES} ${RUNTIME_PATH}/libc/wasmception_backing.c
MINIMAL_CFILES=${RUNTIME_CFILES} ${RUNTIME_PATH}/libc/wasi_sdk_backing.c
MINIMAL_INCLUDES=-I../../runtime/libc/

.PHONY: wasm
wasm: $(patsubst %.wat, %.wasm, $(wildcard *.wat))

.PHONY: bc
bc: $(patsubst %.wat, %.bc, $(wildcard *.wat))

.PHONY: ll
ll: $(patsubst %.wat, %.ll, $(wildcard *.wat))

.PHONY: install_wasmtime
install_wasmtime:
	@curl https://wasmtime.dev/install.sh -sSf | bash

.PHONY: install_wabt
install_wabt:
	@./install_wabt.sh

.PHONY: clean
clean: 
	@rm -f *.wasm *.bc *_wasmception *_uvwasi *_minimal *.ll

%.wasm: %.wat
	# @wat2wasm --debug-names $< -o $@
	@wat2wasm $< -o $@

%.bc: %.wasm
	${AWSM_CC} $< -o $@

%.ll: %.bc
	llvm-dis-12 $< -o $@

.PHONY: %_wasmtime
%_wasmtime: %.wasm
	wasmtime $^

# Using wasmception backend
%_wasmception: %.bc ${WASMCEPTION_RUNTIME_CFILES}
	${CC} -lm ${OPTFLAGS} $^ -o $@

# Using minimal WASI-SDK backend
%_minimal: %.bc ${MINIMAL_CFILES} ../../runtime/thirdparty/dist/include
	clang -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} ${MINIMAL_INCLUDES} ${MINIMAL_CFILES} $< -o $@
