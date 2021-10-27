ROOT_PATH:=$(shell cd ../.. && realpath .)
RUNTIME_PATH:=${ROOT_PATH}/runtime

CC=clang
OPTFLAGS=-O0 -g -flto
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
	align__.minimal.out \
	block__.minimal.out \
	block__break-inner.minimal.out \
	bulk__data.drop.minimal.out \
	bulk__elem.drop.minimal.out \
	bulk__goodbye.minimal.out \
	bulk__memory.copy.minimal.out \
	bulk__memory.fill.minimal.out \
	bulk__memory.init.minimal.out \
	bulk__sixtyfive.minimal.out \
	bulk__table.copy.minimal.out \
	bulk__table.init.minimal.out \
	br__.minimal.out \
	br__as-binary-both.minimal.out \
	br__as-binary-left.minimal.out \
	br__as-block-first.minimal.out \
	br__as-block-mid.minimal.out \
	br__as-block-value-deep.minimal.out \
	br__as-call_indirect-all.minimal.out \
	br__as-call_indirect-first.minimal.out \
	br__as-call_indirect-func.minimal.out \
	br__as-call_indirect-last.minimal.out \
	br__as-call_indirect-mid.minimal.out \
	br__as-call-all.minimal.out \
	br__as-call-first.minimal.out \
	br__as-call-last.minimal.out \
	br__as-call-mid.minimal.out \
	br__as-compare-both.minimal.out \
	br__as-compare-left.minimal.out \
	br__as-compare-right.minimal.out \
	br__as-convert-operand.minimal.out \
	br__as-global.set-value.minimal.out \
	br__as-if-cond.minimal.out \
	br__as-load-address.minimal.out \
	br__as-loadN-address.minimal.out \
	br__as-memory.grow-size.minimal.out \
	br__as-store-address.minimal.out \
	br__as-storeN-address.minimal.out \
	br__as-select-all.minimal.out \
	br__as-select-cond.minimal.out \
	br__as-select-first.minimal.out \
	br__as-select-second.minimal.out \
	br__as-store-both.minimal.out \
	br__as-store-value.minimal.out \
	br__as-storeN-both.minimal.out \
	br__as-storeN-value.minimal.out \
	br__type-f32-f32.minimal.out \
	br__type-f32.minimal.out \
	br__type-f64-f64.minimal.out \
	br__type-f64.minimal.out \
	br__type-i32-i32.minimal.out \
	br__type-i32.minimal.out \
	br__type-i32-value.minimal.out \
	br__type-i64-i64.minimal.out \
	br__type-i64.minimal.out \
	br__type-i64-value.minimal.out \
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
	call__.minimal.out \
	call__fib.minimal.out \
	call_indirect__.minimal.out \
	conversions__.minimal.out \
	conversions__i32.reinterpret_f32.minimal.out \
	conversions__i32.trunc_f32_u.minimal.out \
	conversions__i32.trunc_f64_s.minimal.out \
	conversions__i32.trunc_f64_u.minimal.out \
	conversions__i64.extend_i32_s.minimal.out \
	conversions__i64.extend_i32_u.minimal.out \
	conversions__i64.trunc_f32_u.minimal.out \
	conversions__i64.trunc_f64_u.minimal.out \
	i32__.minimal.out \
	i32__clz.minimal.out \
	i32__ctz.minimal.out \
	i32__rem_s.minimal.out \
	if__.minimal.out \
	if__as-unary-operand.minimal.out \
	loop__.minimal.out \
	return__.minimal.out \
	select__.minimal.out \
	switch__.minimal.out \
	unwind__.minimal.out \
	unwind__by-br_if-value.minimal.out \
	
