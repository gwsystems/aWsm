WASMCC=${WASI_SDK_PATH}/bin/clang --sysroot=${WASI_SDK_PATH}/share/wasi-sysroot
WASMLINKFLAGS=-Wl,--allow-undefined,-z,stack-size=32768,--threads=1
OPTFLAGS=-O3 -g -flto

# Select a runtime
RUNTIME=../../runtime/dist/rt_uvwasi.a
# RUNTIME=../../runtime/dist/rt_minimal.a

OBJ = $(patsubst c/%.c, vm/%_vm, $(wildcard c/*.c))
all: ${OBJ}

../../target/release/awsm:
	cd ../../ && cargo build --release

../../runtime/dist/rt_minimal.a:
	make -C ../../runtime/ dist/rt_minimal.a

../../runtime/dist/rt_uvwasi.a:
	make -C ../../runtime/ dist/rt_uvwasi.a

wasm/%.wasm: c/%.c
	@mkdir -p wasm
	${WASMCC} ${WASMLINKFLAGS} ${OPTFLAGS} $< -o $@

bc/%.bc: wasm/%.wasm
	@mkdir -p bc
	../../target/release/awsm  $< -o $@

vm/%_vm: bc/%.bc ${RUNTIME}
	clang -pthread ${OPTFLAGS} ${RUNTIME} $^ -ldl -lc -lm -o $@

.PHONY clean:
	rm -f ./wasm/* ./bc/* ./vm/*_vm
