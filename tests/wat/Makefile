CC=clang
OPTFLAGS=-O0 -g

# Assuming to be called from ./tests/wat directory
ROOT_PATH:=$(shell cd ../.. && realpath .)
RUNTIME_PATH:=${ROOT_PATH}/runtime

AWSM_CC:=${ROOT_PATH}/target/release/awsm

WASM_BACKING:=wasmception_backing.c

RUNTIME_CFILES+=${RUNTIME_PATH}/runtime.c
RUNTIME_CFILES+=${RUNTIME_PATH}/libc/${WASM_BACKING}
RUNTIME_CFILES+=${RUNTIME_PATH}/libc/env.c
RUNTIME_CFILES+=${RUNTIME_PATH}/memory/64bit_nix.c

clean: if_clean args_clean funcs_clean

# Should work for Linux. Alternate installation options at https://wasmtime.dev/
.PHONY: install_wasmtime
install_wasmtime:
	@curl https://wasmtime.dev/install.sh -sSf | bash

.PHONY: install_wabt
install_wabt:
	@./install_wabt.sh

if.wasm:
	@wat2wasm if.wat -o if.wasm

if.bc: if.wasm
	@RUST_BACKTRACE=1 ${AWSM_CC} ./if.wasm -o ./if.bc

.PHONY: if_wasmtime
if_wasmtime: if.wasm
	@wasmtime if.wasm

if_vm: if.bc
	@${CC} -lm ${OPTFLAGS} \
	./if.bc \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/${WASM_BACKING} \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/64bit_nix.c \
	-o ./if_vm

.PHONY: if_clean
if_clean:
	@rm -f if.wasm if.bc if_vm

args.wasm: args.wat
	@wat2wasm args.wat -o args.wasm

.PHONY: args_wasmtime
args_wasmtime: args.wasm
	@wasmtime args.wasm arg1 arg2 arg3

args.bc: args.wasm
	@RUST_BACKTRACE=1 ${AWSM_CC} ./args.wasm -o ./args.bc

args_vm: args.bc
	@${CC} -lm ${OPTFLAGS} \
	./args.bc \
	${RUNTIME_PATH}/runtime.c \
	${RUNTIME_PATH}/libc/${WASM_BACKING} \
	${RUNTIME_PATH}/libc/env.c \
	${RUNTIME_PATH}/memory/64bit_nix.c \
	-o ./args_vm

.PHONY: args_clean
args_clean:
	@rm -f args.wasm args.bc args_vm

funcs.wasm: funcs.wat
	wat2wasm --debug-names $^ -o $@
	# wat2wasm $^ -o $@

funcs.bc: funcs.wasm
	${AWSM_CC} $^ -o $@

funcs.ll: funcs.bc
	llvm-dis-12 $^ -o $@

funcs_vm: ${RUNTIME_CFILES} ./funcs.bc
	@${CC} -lm ${OPTFLAGS} $^ -o $@

.PHONY: funcs_clean
funcs_clean:
	rm -f funcs.bc funcs.ll funcs.wasm funcs_vm
