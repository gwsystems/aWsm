CC = ../../wasi-sdk/bin/clang
TARGET = wasm32-unknown-wasi
SYSROOT =

# Log all WASI syscalls and arguments to stderr
# RUNTIME_CFLAGS += -DLOG_WASI

OBJ = $(patsubst c/%.c, vm/%_vm, $(wildcard c/*.c))
all: $(OBJ)

wasm/%.wasm: c/%.c
	@mkdir -p wasm
	$(CC) -Wl,--allow-undefined,-z,stack-size=32768,--threads=1 --target=wasm32-wasi -mcpu=mvp -O3 -flto  --sysroot=../../wasi-sdk/share/wasi-sysroot $< -o $@

bc/%.bc: wasm/%.wasm
	@mkdir -p bc
	../../target/release/awsm  $< -o $@

# Using uvwasi backend
vm/%_vm: bc/%.bc ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_impl_uvwasi.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c
	clang -pthread -ldl -lm -O3 $(RUNTIME_CFLAGS) -I../../runtime/libc/wasi/include -I../../runtime/uvwasi/include $< ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_impl_uvwasi.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c ../../runtime/uvwasi/out/cmake/libuvwasi_a.a ../../runtime/libuv/build/libuv_a.a -o $@

# Using minimal backend
# vm/%_vm: bc/%.bc ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_impl_minimal.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c
# 	clang -pthread -ldl -lm -O3 $(RUNTIME_CFLAGS) -I../../runtime/libc/wasi/include -I../../runtime/uvwasi/include $< ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_impl_minimal.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c -o $@

.PHONY clean:
	rm -f ./wasm/* ./bc/* ./vm/*_vm
