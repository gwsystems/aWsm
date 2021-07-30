.PHONY: clean
clean:
	rm -f random.wasm random.bc random_vm

random.wasm: random.c
	../../wasi-sdk/bin/clang -Wl,--allow-undefined,-z,stack-size=32768,--threads=1 --target=wasm32-wasi -mcpu=mvp -O3 -flto --sysroot=../../wasi-sdk/share/wasi-sysroot  -O3 -flto random.c -o random.wasm

random.bc: random.wasm
	../../target/release/awsm  random.wasm -o random.bc

random_vm: random.bc ../../runtime/runtime.c ../../runtime/libc/uvwasi_backing.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c
	clang -pthread -ldl -lm -O0 -g -I../../runtime/uvwasi/include random.bc ../../runtime/runtime.c ../../runtime/libc/uvwasi_backing.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c ../../runtime/uvwasi/out/cmake/libuvwasi_a.a ../../runtime/libuv/build/libuv_a.a -o random_vm
