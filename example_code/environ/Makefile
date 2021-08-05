.PHONY: clean
clean:
	rm -f environ.wasm environ.bc environ_vm

environ.wasm: environ.c
	../../wasi-sdk/bin/clang -Wl,--allow-undefined,-z,stack-size=32768,--threads=1 --target=wasm32-wasi -mcpu=mvp -O3 -flto --sysroot=../../wasi-sdk/share/wasi-sysroot  -O3 -flto environ.c -o environ.wasm

environ.bc: environ.wasm
	../../target/release/awsm  environ.wasm -o environ.bc

environ_vm: environ.bc ../../runtime/runtime.c ../../runtime/libc/wasi_sdk_backing.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c
	clang -lm  -O3 -flto -g environ.bc ../../runtime/runtime.c ../../runtime/libc/wasi_sdk_backing.c ../../runtime/libc/env.c ../../runtime/memory/64bit_nix.c -o environ_vm
