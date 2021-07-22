CC=clang
OPTFLAGS=-O0 -g

ROOT_PATH:=../..

WASM_CC:=${ROOT_PATH}/wasi-sdk/bin/clang
WASI_SDK_SYSROOT:=${ROOT_PATH}/wasi-sdk/share/wasi-sysroot
WASM_FLAGS:=--target=wasm32-wasi -mcpu=mvp ${OPTFLAGS} --sysroot=${WASI_SDK_SYSROOT}
WASM_LINKER_FLAGS:=-Wl,--allow-undefined,--threads=1

lpd.wasm: ./sod/sod.c ./sod/samples/license_plate_detection.c
	@mkdir -p ./app_pid/bin
	${WASM_CC} ${WASM_FLAGS} ${WASM_LINKER_FLAGS} -D_WASI_EMULATED_MMAN -Wl,-z,stack-size=256,-lwasi-emulated-mman  -I./sod -Wall ./sod/sod.c ./sod/samples/license_plate_detection.c -o lpd.wasm

clean:
	rm -rf resize resize.wasm resize.bc resize_vm

resize.wasm: ./sod/sod.c ./sod/samples/resize_image.c
	@mkdir -p ./app_pid/bin
	${WASM_CC} ${WASM_FLAGS} ${WASM_LINKER_FLAGS} -D_WASI_EMULATED_MMAN -Wl,-z -lwasi-emulated-mman -lm -I./sod -Wall ./sod/sod.c ./sod/samples/resize_image.c -o resize.wasm

resize: ./sod/sod.c ./sod/samples/resize_image.c 
	@mkdir -p ./app_pid/bin
	${CC} -I./sod -lm -Wall ./sod/sod.c ./sod/samples/resize_image.c -o resize

resize.bc: resize.wasm
	${ROOT_PATH}/target/release/awsm  resize.wasm -o resize.bc

# Change the optimization level from -O0 to -O1 to trigger the bug
resize_vm: resize.bc ${ROOT_PATH}/runtime/runtime.c ${ROOT_PATH}/runtime/libc/wasi_sdk_backing.c ${ROOT_PATH}/runtime/libc/env.c ${ROOT_PATH}/runtime/memory/64bit_nix.c
	clang -lm -O1 -flto -g resize.bc ${ROOT_PATH}/runtime/runtime.c ${ROOT_PATH}/runtime/libc/wasi_sdk_backing.c ${ROOT_PATH}/runtime/libc/env.c ${ROOT_PATH}/runtime/memory/64bit_nix.c -o resize_vm

.PHONY: resize_wasmtime
resize_wasmtime: resize.wasm
	wasmtime resize.wasm <./sod/samples/flower.jpg >resize.jpg

.PHONY: resize_awsm
resize_awsm: resize_vm
	./resize_vm <./sod/samples/flower.jpg >resize.jpg
