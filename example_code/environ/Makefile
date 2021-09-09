WASMCC=${WASI_SDK_PATH}/bin/clang --sysroot=${WASI_SDK_PATH}/share/wasi-sysroot
WASMLINKFLAGS=-Wl,--allow-undefined,-z,stack-size=32768,--threads=1
OPTFLAGS=-O3 -flto

RUNTIME_INCLUDES=-I../runtime/libc/wasi/include -I../runtime/thirdparty/dist/include

RUNTIME_CPATH+=${RUNTIME_PATH}/runtime.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_main.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_backing.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_impl_uvwasi.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/env.c
RUNTIME_CPATH+=${RUNTIME_PATH}/memory/64bit_nix.c
RUNTIME_CPATH+=${RUNTIME_PATH}/thirdparty/dist/lib/libuv_a.a
RUNTIME_CPATH+=${RUNTIME_PATH}/thirdparty/dist/lib/libuvwasi_a.a

.PHONY: clean
clean:
	rm -f environ.wasm environ.bc environ_vm

environ.wasm: environ.c
	${WASMCC} ${WASMLINKFLAGS} ${OPTFLAGS} $^ -o $@

environ.bc: environ.wasm
	../../target/release/awsm  environ.wasm -o environ.bc

environ_vm: environ.bc ${RUNTIME_CPATH}
	clang -lm -O3 -flto -g ${RUNTIME_INCLUDES} $^ -o $@
