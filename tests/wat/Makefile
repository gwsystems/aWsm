OPTFLAGS=-O3 -flto
WASMLINKFLAGS=-Wl,--allow-undefined,-z,stack-size=32768,--threads=1

# Log all WASI syscalls and arguments to stderr
# RUNTIME_CFLAGS += -DLOG_WASI

.PHONY: all
all: $(patsubst %.wat, %_vm, $(wildcard *.wat))

.PHONY: clean
clean: $(patsubst %.wat, %.clean, $(wildcard *.wat))

%.wasm: %.wat
	@wat2wasm $< -o $@

%.bc: %.wasm
	@../../target/release/awsm $< -o $@

# Using uvwasi backend
%_vm: %.bc ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_impl_uvwasi.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c
	clang -pthread -ldl -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} -I../../runtime/libc/wasi/include -I../../runtime/thirdparty/dist/include $< ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_impl_uvwasi.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c ../../runtime/thirdparty/dist/lib/libuvwasi_a.a ../../runtime/thirdparty/dist/lib/libuv_a.a -o $@

# Using minimal backend
# %_vm: %.bc ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_impl_minimal.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c
# 	clang -pthread -ldl -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} -I../../runtime/libc/wasi/include -I../../runtime/uvwasi/include $< ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_impl_minimal.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c -o $@

.PHONY: %.clean
%.clean:
	rm -f $*.wasm $*.bc $*_vm
