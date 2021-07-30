.PHONY: clean
clean:
	rm -f signal.wasm signal.bc signal_vm

signal.wasm: signal.c
	../../wasi-sdk/bin/clang -Wl,--allow-undefined,-z,stack-size=32768,--threads=1 --target=wasm32-wasi -mcpu=mvp -O0  --sysroot=../../wasi-sdk/share/wasi-sysroot  -O3 -flto signal.c -o signal.wasm

signal.bc: signal.wasm
	../../target/release/awsm  signal.wasm -o signal.bc

signal_vm: signal.bc ../../runtime/runtime.c ../../runtime/libc/uvwasi_backing.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c
	clang -pthread -ldl -lm -O0 -g -I../../runtime/uvwasi/include signal.bc ../../runtime/runtime.c ../../runtime/libc/uvwasi_backing.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c ../../runtime/uvwasi/out/cmake/libuvwasi_a.a ../../runtime/libuv/build/libuv_a.a -o signal_vm
