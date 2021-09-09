ROOT_PATH:=$(shell cd ../.. && realpath .)
RUNTIME_PATH:=${ROOT_PATH}/runtime

CC=clang
OPTFLAGS=-O3 -flto
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
	@rm -f *.wasm *.bc *_wasmception *_uvwasi *_minimal *.ll

%.wasm: %.wat
	@wat2wasm --debug-names $< -o $@

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

# Using uvwasi backend
../../runtime/thirdparty/dist/lib/libuv_a.a:
	make -C ../../runtime/thirdparty libuv.install

../../runtime/thirdparty/dist/lib/libuvwasi_a.a:
	make -C ../../runtime/thirdparty dist/lib/libuvwasi_a.a:

.PHONY: ../../runtime/thirdparty/dist/include
../../runtime/thirdparty/dist/include:
	make -C ../../runtime/thirdparty dist/include/wasi

UVWASI_CFILES=../../runtime/runtime.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_impl_uvwasi.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c
UVWASI_LIBS=../../runtime/thirdparty/dist/lib/libuvwasi_a.a ../../runtime/thirdparty/dist/lib/libuv_a.a 
UVWASI_INCLUDES=-I../../runtime/libc/wasi/include -I../../runtime/thirdparty/dist/include

%_uvwasi: %.bc ${UVWASI_CFILES} ${UVWASI_LIBS}
	clang -pthread -ldl -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} ${UVWASI_INCLUDES} $^ -o $@

# Using minimal backend
MINIMAL_CFILES=${RUNTIME_CFILES} ${RUNTIME_PATH}/libc/wasi/wasi_backing.c ${RUNTIME_PATH}/libc/wasi/wasi_main.c ${RUNTIME_PATH}/libc/wasi/wasi_impl_minimal.c
MINIMAL_INCLUDES=-I${RUNTIME_PATH}/libc/

%_minimal: %.bc ${MINIMAL_CFILES} ../../runtime/thirdparty/dist/include
	clang -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} ${MINIMAL_INCLUDES} ${MINIMAL_CFILES} $< -o $@
