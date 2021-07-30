.PHONY: clean
clean:
	rm -f time.wasm time.bc time_vm

time.wasm: time.c
	../../wasi-sdk/bin/clang -Wl,--allow-undefined,-z,stack-size=32768,--threads=1 --target=wasm32-wasi -mcpu=mvp -O3 -flto --sysroot=../../wasi-sdk/share/wasi-sysroot  -O3 -flto time.c -o time.wasm

time.bc: time.wasm
	../../target/release/awsm  time.wasm -o time.bc

time_vm: time.bc ../../runtime/runtime.c ../../runtime/libc/uvwasi_backing.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c
	clang -pthread -ldl -lm -O0 -g -I../../runtime/uvwasi/include time.bc ../../runtime/runtime.c ../../runtime/libc/uvwasi_backing.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c ../../runtime/uvwasi/out/cmake/libuvwasi_a.a ../../runtime/libuv/build/libuv_a.a -o time_vm
