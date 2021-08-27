WASMCC=/opt/wasi-sdk/bin/clang
WASMLINKFLAGS=-Wl,--allow-undefined,-z,stack-size=32768,--threads=1
OPTFLAGS=-O3 -flto

RUNTIME_INCLUDES=-I../runtime/libc/wasi/include -I../runtime/thirdparty/dist/include

RUNTIME_CPATH+=${RUNTIME_PATH}/runtime.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_main.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_backing.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_impl_uvwasi.c
RUNTIME_CPATH+=${RUNTIME_PATH}/libc/env.c
RUNTIME_CPATH+=${RUNTIME_PATH}/memory/64bit_nix.c
RUNTIME_CPATH+=${RUNTIME_PATH}/thirdparty/dist/lib/libuv_a.a
RUNTIME_CPATH+=${RUNTIME_PATH}/thirdparty/dist/lib/libuvwasi_a.a

bin/pb_la_blas_gemm.wasm: gemm.c polybench.c
	${WASMCC} ${WASMLINKFLAGS} ${OPTFLAGS} $^ -o $@

bin/pb_la_blas_gemm.bc: bin/pb_la_blas_gemm.wasm
	../../target/release/awsm  bin/pb_la_blas_gemm.wasm -o bin/pb_la_blas_gemm.bc

bin/pb_la_blas_gemm: gemm.c polybench.c
	clang -lm  -O3 -flto -g gemm.c polybench.c -o bin/pb_la_blas_gemm

bin/pb_la_blas_gemm_cm: bin/pb_la_blas_gemm.bc ../../runtime/runtime.c ../../runtime/libc/wasi_sdk_backing.c ../../runtime/libc/env.c ../../runtime/memory/cortex_m.c
	clang -lm  -O3 -flto -g bin/pb_la_blas_gemm.bc ../../runtime/runtime.c ../../runtime/libc/wasi_sdk_backing.c ../../runtime/libc/env.c ../../runtime/memory/cortex_m.c -o bin/pb_la_blas_gemm_cm

bin/pb_la_blas_gemm_np_us: bin/pb_la_blas_gemm_us.bc ../../runtime/runtime.c ../../runtime/libc/wasi_sdk_backing.c ../../runtime/libc/env.c ../../runtime/memory/no_protection.c
	clang -lm  -O3 -flto -g bin/pb_la_blas_gemm_us.bc ../../runtime/runtime.c ../../runtime/libc/wasi_sdk_backing.c ../../runtime/libc/env.c ../../runtime/memory/no_protection.c -o bin/pb_la_blas_gemm_np_us

bin/pb_la_blas_gemm_np: bin/pb_la_blas_gemm.bc ../../runtime/runtime.c ../../runtime/libc/wasi_sdk_backing.c ../../runtime/libc/env.c ../../runtime/memory/no_protection.c
	clang -lm  -O3 -flto -g bin/pb_la_blas_gemm.bc ../../runtime/runtime.c ../../runtime/libc/wasi_sdk_backing.c ../../runtime/libc/env.c ../../runtime/memory/no_protection.c -o bin/pb_la_blas_gemm_np

bin/pb_la_blas_gemm_bc: bin/pb_la_blas_gemm.bc ../../runtime/runtime.c ../../runtime/libc/wasi_sdk_backing.c ../../runtime/libc/env.c ../../runtime/memory/generic.c
	clang -lm  -O3 -flto -g bin/pb_la_blas_gemm.bc ../../runtime/runtime.c ../../runtime/libc/wasi_sdk_backing.c ../../runtime/libc/env.c ../../runtime/memory/generic.c -o bin/pb_la_blas_gemm_bc

bin/pb_la_blas_gemm_vm: bin/pb_la_blas_gemm.bc ${RUNTIME_CPATH}
	clang -lm ${RUNTIME_INCLUDES} ${OPTFLAGS} -g $^ -o $@
