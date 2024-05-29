WASMCC=${WASI_SDK_PATH}/bin/clang --sysroot=${WASI_SDK_PATH}/share/wasi-sysroot
WASMLINKFLAGS=-Wl,--allow-undefined,-z,stack-size=32768,--threads=1
OPTFLAGS=-O0 -g
ARCH=64bit_nix

# Log all WASI syscalls and arguments to stderr
# RUNTIME_CFLAGS += -DLOG_WASI


# program.c => wasi-sdk => program.wasm => aWsm => program.bc => clang => program (runtime baked in)

OBJ = $(patsubst c/%.c, vm/%_vm, $(wildcard c/*.c))
all: ${OBJ}

.PRECIOUS: wasm/%_original.wasm
wasm/%_original.wasm: c/%.c
	@mkdir -p wasm
	${WASMCC} ${WASMLINKFLAGS} ${OPTFLAGS} $< -o $@

.PRECIOUS: wasm/%.wasm
wasm/%.wasm: wasm/%_original.wasm
	wasm-opt -O2 --no-exit-runtime $< -o $@

wasm/%.wat: wasm/%.wasm
	wasm-opt --print $< > $@

wasm/%_metrics.txt: wasm/%.wasm
	wasm-opt --metrics $< >$@

wasm/%_func_metrics.txt: wasm/%.wasm
	wasm-opt --func-metrics $< >$@

wasm/%_call_graph.dot: wasm/%.wasm
	wasm-opt --print-call-graph $< >$@

wasm/%_call_graph.svg: wasm/%_call_graph.dot
	dot -Tsvg $< >$@

debug: wasm/atof_metrics.txt \
	wasm/atof_original_metrics.txt \
	wasm/atof_func_metrics.txt \
	wasm/atof_original_func_metrics.txt \
	wasm/atof_call_graph.dot \
	wasm/atof_call_graph.svg \
	wasm/atof_original_call_graph.dot \
	wasm/atof_original_call_graph.svg \
	wasm/atof.wat \
	wasm/atof_original.wat

%.wasi: %.wasm
	wasm2wat $^ -o $@

bc/%.bc: wasm/%.wasm
	@mkdir -p bc
	../../target/debug/awsm  $< -o $@

# Using uvwasi backend

../../runtime/thirdparty/dist/lib/libuvwasi_a.a:
	make -C ../../runtime/thirdparty uvwasi.install
../../runtime/thirdparty/dist/lib/libuv_a.a:
	make -C ../../runtime/thirdparty libuv.install

UVWASI_CFILES=../../runtime/runtime.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_impl_uvwasi.c ../../runtime/libc/env.c ../../runtime/memory/${ARCH}.c

UVWASI_LIBS=../../runtime/thirdparty/dist/lib/libuvwasi_a.a ../../runtime/thirdparty/dist/lib/libuv_a.a 

UVWASI_INCLUDES=-I../../runtime/libc/wasi/include -I../../runtime/thirdparty/dist/include

WASM_INSTRUCTIONS=../../runtime/wasm_instructions/common.c ../../runtime/wasm_instructions/memory/${ARCH}.c

vm/%_vm: bc/%.bc ${UVWASI_CFILES} ${UVWASI_LIBS} ${WASM_INSTRUCTIONS}
	clang -pthread -ldl -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} ${UVWASI_INCLUDES} $^ -o $@

# Using minimal backend
# vm/%_vm: bc/%.bc ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_impl_minimal.c ../../runtime/libc/env.c ../../runtime/memory/${ARCH}.c
# 	clang -pthread -ldl -lm ${OPTFLAGS} ${RUNTIME_CFLAGS} -I../../runtime/libc/wasi/include -I../../runtime/uvwasi/include $< ../../runtime/runtime.c ../../runtime/libc/wasi/wasi_main.c ../../runtime/libc/wasi/wasi_backing.c ../../runtime/libc/wasi/wasi_impl_minimal.c ../../runtime/libc/env.c ../../runtime/memory/${ARCH}.c -o $@

.PHONY clean:
	rm -f ./wasm/* ./bc/* ./vm/*_vm
