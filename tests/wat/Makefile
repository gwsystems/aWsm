ROOT_PATH:=$(shell cd ../.. && realpath .)
RUNTIME_PATH:=${ROOT_PATH}/runtime

CC=clang
OPTFLAGS=-O0 -g -flto
AWSM_CC:=${ROOT_PATH}/target/debug/awsm

RUNTIME_CFILES+=${RUNTIME_PATH}/runtime.c
RUNTIME_CFILES+=${RUNTIME_PATH}/libc/env.c
RUNTIME_CFILES+=${RUNTIME_PATH}/memory/64bit_nix.c

WASMCEPTION_RUNTIME_CFILES=${RUNTIME_CFILES} ${RUNTIME_PATH}/libc/wasmception_backing.c

WASI_COMMON_CFILES=${RUNTIME_CFILES} ${RUNTIME_PATH}/libc/wasi/wasi_backing.c ${RUNTIME_PATH}/libc/wasi/wasi_main.c 

MINIMAL_CFILES=${WASI_COMMON_CFILES} ${RUNTIME_PATH}/libc/wasi/wasi_impl_minimal.c
MINIMAL_INCLUDES=-I${RUNTIME_PATH}/libc/ -I${RUNTIME_PATH}/libc/wasi/include/

UVWASI_CFILES=${WASI_COMMON_CFILES} ${RUNTIME_PATH}/libc/wasi/wasi_impl_uvwasi.c
UVWASI_LIBS=${RUNTIME_PATH}/thirdparty/dist/lib/libuvwasi_a.a ${RUNTIME_PATH}/thirdparty/dist/lib/libuv_a.a 
UVWASI_INCLUDES=-I${RUNTIME_PATH}/libc/wasi/include -I${RUNTIME_PATH}/thirdparty/dist/include

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
	@rm -f *.wasm *.bc *.out *.ll *.log

%.wasm: %.wat
	@wat2wasm --debug-names $< -o $@

%.bc: %.wasm
	@RUST_BACKTRACE=1 ${AWSM_CC} $< -o $@ 1>$*.log 2>&1

%.ll: %.bc
	@llvm-dis-12 $< -o $@

.PHONY: %_wasmtime
%_wasmtime: %.wasm
	@wasmtime $^

# Using wasmception backend
%.wasmception.out: %.bc ${WASMCEPTION_RUNTIME_CFILES}
	@${CC} -lm ${OPTFLAGS} $^ -o $@

# Using uvwasi backend
.PHONY: ../../runtime/thirdparty/dist/lib/libuv_a.a
../../runtime/thirdparty/dist/lib/libuv_a.a:
	@echo "Installing libuv"
	@make -C ../../runtime/thirdparty libuv.install

.PHONY: ../../runtime/thirdparty/dist/lib/libuvwasi_a.a
../../runtime/thirdparty/dist/lib/libuvwasi_a.a:
	@echo "Installing uvwasi"
	@make -C ../../runtime/thirdparty uvwasi.install

%.uvwasi.out: %.bc ${UVWASI_CFILES} ${UVWASI_LIBS}
	@${CC} -pthread -ldl -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} ${UVWASI_INCLUDES} $^ -o $@

%.minimal.out: %.bc ${MINIMAL_CFILES} ../../runtime/thirdparty/dist/include
	@${CC} -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} ${MINIMAL_INCLUDES} ${MINIMAL_CFILES} $< -o $@

.PHONY: uvwasi.out
uvwasi.out: $(patsubst %.wat, %.uvwasi.out, $(wildcard *.wat))

.PHONY: minimal.out
minimal.out: $(patsubst %.wat, %.minimal.out, $(wildcard *.wat))

# Wasmception can only build tests that don't use wasi imports
TESTS_WITHOUT_WASI_IMPORTS:=$(shell grep -L wasi_ *.wat)

.PHONY: wasmception.out
wasmception.out: $(patsubst %.wat, %.wasmception.out, ${TESTS_WITHOUT_WASI_IMPORTS})

all: uvwasi.out minimal.out wasmception.out
