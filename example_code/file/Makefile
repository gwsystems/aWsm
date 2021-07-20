.PHONY: clean
clean:
	rm -f file.wasm file.bc file_vm

file.wasm: file.c
	../../wasi-sdk/bin/clang -Wl,--allow-undefined,-z,stack-size=32768,--threads=1 --target=wasm32-wasi -mcpu=mvp -O3 -flto --sysroot=../../wasi-sdk/share/wasi-sysroot  -O3 -flto file.c -o file.wasm

file.bc: file.wasm
	../../target/release/awsm  file.wasm -o file.bc

file_vm: file.bc ../../runtime/runtime.c ../../runtime/libc/wasi_sdk_backing.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c
	clang -lm  -O3 -flto -g file.bc ../../runtime/runtime.c ../../runtime/libc/wasi_sdk_backing.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c -o file_vm
	