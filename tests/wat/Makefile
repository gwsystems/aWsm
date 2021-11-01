ROOT_PATH:=$(shell cd ../.. && realpath .)
RUNTIME_PATH:=${ROOT_PATH}/runtime

CC=clang
OPTFLAGS=-O0 -g -flto
AWSM_CC:=${ROOT_PATH}/target/debug/awsm

RUNTIME_CFILES+=${RUNTIME_PATH}/runtime.c
RUNTIME_CFILES+=${RUNTIME_PATH}/libc/env.c
RUNTIME_CFILES+=${RUNTIME_PATH}/memory/64bit_nix.c

WASMCEPTION_RUNTIME_CFILES=${RUNTIME_CFILES} ${RUNTIME_PATH}/libc/wasmception_backing.c

.PHONY: wasm
wasm: $(patsubst %.wat, %.wasm, $(wildcard *.wat))

.PHONY: bc
bc: $(patsubst %.wat, %.bc, $(wildcard *.wat))

.PHONY: ll
ll: $(patsubst %.wat, %.ll, $(wildcard *.wat))

# Log all WASI syscalls and arguments to stderr
# RUNTIME_CFLAGS += -DLOG_WASI

# modules with WASI imports will fail on wasmception builds
# Only run this manually
# .PHONY: wasmception
# wasmception: $(patsubst %.wat, %_wasmception, $(wildcard *.wat))

.PHONY: uvwasi
uvwasi: $(patsubst %.wat, %_uvwasi, $(wildcard *.wat))

.PHONY: minimal
minimal: $(patsubst %.wat, %_minimal, $(wildcard *.wat))

.PHONY: install_wasmtime
install_wasmtime:
	@curl https://wasmtime.dev/install.sh -sSf | bash

.PHONY: install_wabt
install_wabt:
	@./install_wabt.sh

.PHONY: clean
clean: 
	@rm -f *.wasm *.bc *_wasmception *.out *.ll *.log

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

# Using uvwasi backend
../../runtime/thirdparty/dist/lib/libuv_a.a:
	make -C ../../runtime/thirdparty libuv.install

../../runtime/thirdparty/dist/lib/libuvwasi_a.a:
	make -C ../../runtime/thirdparty uvwasi.install

UVWASI_CFILES=../../runtime/runtime.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_impl_uvwasi.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c
UVWASI_LIBS=../../runtime/thirdparty/dist/lib/libuvwasi_a.a ../../runtime/thirdparty/dist/lib/libuv_a.a 
UVWASI_INCLUDES=-I../../runtime/libc/wasi/include -I../../runtime/thirdparty/dist/include

%.uvwasi.out: %.bc ${UVWASI_CFILES} ${UVWASI_LIBS}
	clang -pthread -ldl -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} ${UVWASI_INCLUDES} $^ -o $@

# Using minimal backend
MINIMAL_CFILES=${RUNTIME_CFILES} ${RUNTIME_PATH}/libc/wasi/wasi_backing.c ${RUNTIME_PATH}/libc/wasi/wasi_main.c ${RUNTIME_PATH}/libc/wasi/wasi_impl_minimal.c
MINIMAL_INCLUDES=-I${RUNTIME_PATH}/libc/ -I${RUNTIME_PATH}/libc/wasi/include/

# Using minimal WASI-SDK backend
%.minimal.out: %.bc ${MINIMAL_CFILES} ../../runtime/thirdparty/dist/include
	clang -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} ${MINIMAL_INCLUDES} ${MINIMAL_CFILES} $< -o $@

TESTING_BACKEND=uvwasi

all: \
	align__.${TESTING_BACKEND}.out \
	block__.${TESTING_BACKEND}.out \
	block__break-inner.${TESTING_BACKEND}.out \
	br__.${TESTING_BACKEND}.out \
	br__as-binary-both.${TESTING_BACKEND}.out \
	br__as-binary-left.${TESTING_BACKEND}.out \
	br__as-block-first.${TESTING_BACKEND}.out \
	br__as-block-mid.${TESTING_BACKEND}.out \
	br__as-block-value-deep.${TESTING_BACKEND}.out \
	br__as-call_indirect-all.${TESTING_BACKEND}.out \
	br__as-call_indirect-first.${TESTING_BACKEND}.out \
	br__as-call_indirect-func.${TESTING_BACKEND}.out \
	br__as-call_indirect-last.${TESTING_BACKEND}.out \
	br__as-call_indirect-mid.${TESTING_BACKEND}.out \
	br__as-call-all.${TESTING_BACKEND}.out \
	br__as-call-first.${TESTING_BACKEND}.out \
	br__as-call-last.${TESTING_BACKEND}.out \
	br__as-call-mid.${TESTING_BACKEND}.out \
	br__as-compare-both.${TESTING_BACKEND}.out \
	br__as-compare-left.${TESTING_BACKEND}.out \
	br__as-compare-right.${TESTING_BACKEND}.out \
	br__as-convert-operand.${TESTING_BACKEND}.out \
	br__as-global.set-value.${TESTING_BACKEND}.out \
	br__as-if-cond.${TESTING_BACKEND}.out \
	br__as-load-address.${TESTING_BACKEND}.out \
	br__as-loadN-address.${TESTING_BACKEND}.out \
	br__as-memory.grow-size.${TESTING_BACKEND}.out \
	br__as-store-address.${TESTING_BACKEND}.out \
	br__as-storeN-address.${TESTING_BACKEND}.out \
	br__as-select-all.${TESTING_BACKEND}.out \
	br__as-select-cond.${TESTING_BACKEND}.out \
	br__as-select-first.${TESTING_BACKEND}.out \
	br__as-select-second.${TESTING_BACKEND}.out \
	br__as-store-both.${TESTING_BACKEND}.out \
	br__as-store-value.${TESTING_BACKEND}.out \
	br__as-storeN-both.${TESTING_BACKEND}.out \
	br__as-storeN-value.${TESTING_BACKEND}.out \
	br__type-f32-f32.${TESTING_BACKEND}.out \
	br__type-f32.${TESTING_BACKEND}.out \
	br__type-f64-f64.${TESTING_BACKEND}.out \
	br__type-f64.${TESTING_BACKEND}.out \
	br__type-i32-i32.${TESTING_BACKEND}.out \
	br__type-i32.${TESTING_BACKEND}.out \
	br__type-i32-value.${TESTING_BACKEND}.out \
	br__type-i64-i64.${TESTING_BACKEND}.out \
	br__type-i64.${TESTING_BACKEND}.out \
	br__type-i64-value.${TESTING_BACKEND}.out \
	br_if__.${TESTING_BACKEND}.out \
	br_table__.${TESTING_BACKEND}.out \
	br_table__as-block-first.${TESTING_BACKEND}.out \
	br_table__as-block-mid.${TESTING_BACKEND}.out \
	br_table__as-call_indirect-first.${TESTING_BACKEND}.out \
	br_table__as-call_indirect-func.${TESTING_BACKEND}.out \
	br_table__as-call_indirect-last.${TESTING_BACKEND}.out \
	br_table__as-call_indirect-mid.${TESTING_BACKEND}.out \
	br_table__as-call-first.${TESTING_BACKEND}.out \
	br_table__as-call-last.${TESTING_BACKEND}.out \
	br_table__as-call-mid.${TESTING_BACKEND}.out \
	br_table__as-compare-left.${TESTING_BACKEND}.out \
	br_table__as-compare-right.${TESTING_BACKEND}.out \
	br_table__as-convert-operand.${TESTING_BACKEND}.out \
	br_table__as-global.set-value.${TESTING_BACKEND}.out \
	br_table__as-if-cond.${TESTING_BACKEND}.out \
	br_table__as-load-address.${TESTING_BACKEND}.out \
	br_table__as-loadN-address.${TESTING_BACKEND}.out \
	br_table__as-loop-first.${TESTING_BACKEND}.out \
	br_table__as-loop-last.${TESTING_BACKEND}.out \
	br_table__as-loop-mid.${TESTING_BACKEND}.out \
	br_table__as-memory.grow-size.${TESTING_BACKEND}.out \
	br_table__as-select-first.${TESTING_BACKEND}.out \
	br_table__as-select-second.${TESTING_BACKEND}.out \
	br_table__as-store-address.${TESTING_BACKEND}.out \
	br_table__as-store-value.${TESTING_BACKEND}.out \
	br_table__as-storeN-address.${TESTING_BACKEND}.out \
	br_table__as-storeN-value.${TESTING_BACKEND}.out \
	br_table__type-f32-value.${TESTING_BACKEND}.out \
	br_table__type-f32.${TESTING_BACKEND}.out \
	br_table__type-f64.${TESTING_BACKEND}.out \
	br_table__type-i32-value.${TESTING_BACKEND}.out \
	br_table__type-i32.${TESTING_BACKEND}.out \
	br_table__type-i64-value.${TESTING_BACKEND}.out \
	br_table__type-i64.${TESTING_BACKEND}.out \
	call__.${TESTING_BACKEND}.out \
	call__fib.${TESTING_BACKEND}.out \
	call_indirect__.${TESTING_BACKEND}.out \
	conversions__.${TESTING_BACKEND}.out \
	conversions__i32.reinterpret_f32.${TESTING_BACKEND}.out \
	conversions__i32.trunc_f32_u.${TESTING_BACKEND}.out \
	conversions__i32.trunc_f64_s.${TESTING_BACKEND}.out \
	conversions__i32.trunc_f64_u.${TESTING_BACKEND}.out \
	conversions__i64.extend_i32_s.${TESTING_BACKEND}.out \
	conversions__i64.extend_i32_u.${TESTING_BACKEND}.out \
	conversions__i64.trunc_f32_u.${TESTING_BACKEND}.out \
	conversions__i64.trunc_f64_u.${TESTING_BACKEND}.out \
	i32__.${TESTING_BACKEND}.out \
	i32__clz.${TESTING_BACKEND}.out \
	i32__ctz.${TESTING_BACKEND}.out \
	i32__rem_s.${TESTING_BACKEND}.out \
	i64__.${TESTING_BACKEND}.out \
	i64__ctz.${TESTING_BACKEND}.out \
	i64__rem_s.${TESTING_BACKEND}.out \
	if__.${TESTING_BACKEND}.out \
	if__as-unary-operand.${TESTING_BACKEND}.out \
	loop__.${TESTING_BACKEND}.out \
	return__.${TESTING_BACKEND}.out \
	select__.${TESTING_BACKEND}.out \
	switch__.${TESTING_BACKEND}.out \
	unwind__.${TESTING_BACKEND}.out \
	unwind__by-br_if-value.${TESTING_BACKEND}.out \
	
