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
	@rm -f *.wasm *.bc *_wasmception *_uvwasi *.minimal.out *.ll *.log

%.wasm: %.wat
	@wat2wasm --debug-names $< -o $@

%.bc: %.wasm
	RUST_BACKTRACE=1 ${AWSM_CC} $< -o $@ 2> $*.log

%.ll: %.bc
	llvm-dis-12 $< -o $@

.PHONY: %_wasmtime
%_wasmtime: %.wasm
	wasmtime $^

# Using wasmception backend
%.wasmception.out: %.bc ${WASMCEPTION_RUNTIME_CFILES}
	${CC} -lm ${OPTFLAGS} $^ -o $@

# Using minimal WASI-SDK backend
%.minimal.out: %.bc ${MINIMAL_CFILES}
	clang -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} ${MINIMAL_INCLUDES} ${MINIMAL_CFILES} $< -o $@

all: \
	block__.minimal.out \
	block__break-inner.minimal.out \
	br__.minimal.out \
	br__type-f32-f32.minimal.out \
	br__type-f32.minimal.out \
	br__type-f64-f64.minimal.out \
	br__type-f64.minimal.out \
	br__type-i32-i32.minimal.out \
	br__type-i32.minimal.out \
	br__type-i64-i64.minimal.out \
	br__type-i64.minimal.out \
	br_if__.minimal.out \
	br_table__.minimal.out \
	br_table__as-block-first.minimal.out \
	br_table__as-block-mid.minimal.out \
	br_table__as-call_indirect-first.minimal.out \
	br_table__as-call_indirect-func.minimal.out \
	br_table__as-call_indirect-last.minimal.out \
	br_table__as-call_indirect-mid.minimal.out \
	br_table__as-call-first.minimal.out \
	br_table__as-call-last.minimal.out \
	br_table__as-call-mid.minimal.out \
	br_table__as-compare-left.minimal.out \
	br_table__as-compare-right.minimal.out \
	br_table__as-convert-operand.minimal.out \
	br_table__as-global.set-value.minimal.out \
	br_table__as-if-cond.minimal.out \
	br_table__as-load-address.minimal.out \
	br_table__as-loadN-address.minimal.out \
	br_table__as-loop-first.minimal.out \
	br_table__as-loop-last.minimal.out \
	br_table__as-loop-mid.minimal.out \
	br_table__as-memory.grow-size.minimal.out \
	br_table__as-select-first.minimal.out \
	br_table__as-select-second.minimal.out \
	br_table__as-store-address.minimal.out \
	br_table__as-store-value.minimal.out \
	br_table__as-storeN-address.minimal.out \
	br_table__as-storeN-value.minimal.out \
	br_table__type-f32-value.minimal.out \
	br_table__type-f32.minimal.out \
	br_table__type-f64.minimal.out \
	br_table__type-i32-value.minimal.out \
	br_table__type-i32.minimal.out \
	br_table__type-i64-value.minimal.out \
	br_table__type-i64.minimal.out \
	loop__.minimal.out
