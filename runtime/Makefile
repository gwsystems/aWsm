CC=clang
CFLAGS=-fPIC -c -O3 -g

# The purpose of this makefile is to compile the uvwasi and minimal runtimes
# into static archives. This helps simplify linking the runtime with LLVM bitcode
# emited by aWsm and makes our toolchain a bit more approachable.
# TODO: Add rules that compile libuvwasi and libuv


# Log all WASI syscalls and arguments to stderr
# CFLAGS += -DLOG_WASI

INCLUDES=-I. -I./libc/wasi/include -I./thirdparty/dist/include

AWSM_UVWASI_CSRC=./runtime.c ./libc/wasi/wasi_backing.c ./libc/wasi/wasi_main.c ./libc/wasi/wasi_impl_uvwasi.c ./libc/env.c ./memory/64bit_nix.c

AWSM_UVWASI_OBJ=$(patsubst %.c, %.o, $(AWSM_UVWASI_CSRC))

AWSM_UVWASI_DEP=./thirdparty/dist/lib/libuvwasi_a.a ./thirdparty/dist/lib/libuv_a.a

AWSM_MINIMAL_CSRC=./runtime.c ./libc/wasi/wasi_backing.c ./libc/wasi/wasi_main.c ./libc/wasi/wasi_impl_minimal.c ./libc/env.c ./memory/64bit_nix.c

AWSM_MINIMAL_OBJ=$(patsubst %.c, %.o, $(AWSM_MINIMAL_CSRC))

all: dist/rt_uvwasi.a dist/rt_minimal.a

thirdparty/dist/lib/libuv_a.a:
	make -C thirdparty dist/lib/libuv_a.a

thirdparty/dist/lib/libuvwasi_a.a:
	make -C thirdparty dist/lib/libuvwasi_a.a

thirdparty/dist/include/uvwasi.h:
	make -C thirdparty dist/include/uvwasi.h

%.o: %.c
	${CC} ${CFLAGS} -o $@ ${INCLUDES} $< 

dist/rt_uvwasi.a: thirdparty/dist/include/uvwasi.h ${AWSM_UVWASI_DEP} ${AWSM_UVWASI_OBJ} 
	mkdir -p dist
	llvm-ar cqL $@ $^

dist/rt_minimal.a: ${AWSM_MINIMAL_OBJ}
	mkdir -p dist
	llvm-ar cqL $@ $^

clean: 
	rm -f ${AWSM_UVWASI_OBJ} ${AWSM_MINIMAL_OBJ}
	rm -rf ./dist
