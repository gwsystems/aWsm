WASMCC=${WASI_SDK_PATH}/bin/clang --sysroot=${WASI_SDK_PATH}/share/wasi-sysroot
WASMLINKFLAGS=-Wl,--allow-undefined,-z,stack-size=32768,--threads=1
OPTFLAGS=-O0 -flto

# Log all WASI syscalls and arguments to stderr
# RUNTIME_CFLAGS += -DLOG_WASI


# program.c => wasi-sdk-12 => program.wasm => aWsm => program.bc => clang => program (runtime baked in)

OBJ = $(patsubst c/%.c, vm/%_vm, $(wildcard c/*.c))
all: ${OBJ}

wasm/%.wasm: c/%.c
	@mkdir -p wasm
	${WASMCC} ${WASMLINKFLAGS} ${OPTFLAGS} $< -o $@

bc/%.bc: wasm/%.wasm
	@mkdir -p bc
	../../target/release/awsm  $< -o $@

# Using uvwasi backend
vm/%_vm: bc/%.bc ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_impl_uvwasi.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c
	clang -pthread -ldl -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} -I../../runtime/libc/wasi/include -I../../runtime/thirdparty/dist/include $< ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_impl_uvwasi.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c ../../runtime/thirdparty/dist/lib/libuvwasi_a.a ../../runtime/thirdparty/dist/lib/libuv_a.a -o $@

# Using minimal backend
# vm/%_vm: bc/%.bc ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_impl_minimal.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c
# 	clang -pthread -ldl -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} -I../../runtime/libc/wasi/include -I../../runtime/uvwasi/include $< ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_impl_minimal.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c -o $@

.PHONY clean:
	rm -f ./wasm/* ./bc/* ./vm/*_vm
