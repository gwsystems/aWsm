AWSMCC=../target/release/awsm
CC=clang

# Used by aWsm when compiling the *.wasm to *.bc
AWSMFLAGS= --inline-constant-globals

RUNTIME_PATH=../runtime
RUNTIME_INCLUDES=-I${RUNTIME_PATH}/libc/wasi/include -I${RUNTIME_PATH}/thirdparty/dist/include

WASI_CPATH+=${RUNTIME_PATH}/runtime.c
WASI_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_main.c
WASI_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_backing.c
WASI_CPATH+=${RUNTIME_PATH}/libc/wasi/wasi_impl_uvwasi.c
WASI_CPATH+=${RUNTIME_PATH}/libc/env.c
WASI_CPATH+=${RUNTIME_PATH}/memory/64bit_nix.c
WASI_CPATH+=${RUNTIME_PATH}/thirdparty/dist/lib/libuv_a.a
WASI_CPATH+=${RUNTIME_PATH}/thirdparty/dist/lib/libuvwasi_a.a

dist:
	mkdir dist

.PHONY: clean
clean:
	@make -C wasm_apps clean
	@rm -rf dist

wasm_apps/dist/%.wasm:
	make -C wasm_apps $(addprefix dist/,$(notdir $@))

dist/%.bc: ./wasm_apps/dist/%.wasm dist
	${AWSMCC} ${AWSMFLAGS} $< -o $@

dist/%.ll: dist/%.bc
	llvm-dis-12 $< -o $@

dist/armstrong-numbers.awsm: dist/armstrong-numbers.bc armstrong-numbers/main.c ${RUNTIME_PATH}/runtime.c ${RUNTIME_PATH}/memory/64bit_nix.c
	${CC} -lm -O3 -flto $^ -o $@

dist/bank-account.awsm: dist/bank-account.bc bank-account/main.c ${RUNTIME_PATH}/runtime.c ${RUNTIME_PATH}/memory/64bit_nix.c
	${CC} -lm -O3 -flto $^ -o $@

dist/collatz-conjecture.awsm: dist/collatz-conjecture.bc collatz-conjecture/main.c ${RUNTIME_PATH}/runtime.c ${RUNTIME_PATH}/memory/64bit_nix.c
	${CC} -lm -O3 -flto $^ -o $@

dist/darts.awsm: dist/darts.bc darts/main.c ${RUNTIME_PATH}/runtime.c ${RUNTIME_PATH}/memory/64bit_nix.c
	${CC} -lm -O3 -flto $^ -o $@

dist/difference-of-squares.awsm: dist/difference-of-squares.bc difference-of-squares/main.c ${RUNTIME_PATH}/runtime.c ${RUNTIME_PATH}/memory/64bit_nix.c
	${CC} -lm -O3 -flto $^ -o $@

dist/%.awsm: dist/%.bc ${WASI_CPATH}
	${CC} -pthread -ldl -lm -O3 -flto -g ${RUNTIME_INCLUDES} $^ -o $@

.PHONY: all.awsm
all.awsm: \
	dist/app_pid.awsm \
	dist/app_tinycrypt.awsm \
	dist/app_v9.awsm \
	dist/cifar10.awsm \
	dist/custom_binarytrees.awsm \
	dist/custom_function_pointers.awsm \
	dist/custom_matrix_multiply.awsm \
	dist/custom_memcmp.awsm \
	dist/echo.awsm \
	dist/empty.awsm \
	dist/exit.awsm \
	dist/fibonacci.awsm \
	dist/gocr.awsm \
	dist/mi_adpcm.awsm \
	dist/mi_basic_math.awsm \
	dist/mi_bitcount.awsm \
	dist/mi_bitcount_cm.awsm \
	dist/mi_blowfish.awsm \
	dist/mi_crc.awsm \
	dist/mi_dijkstra.awsm \
	dist/mi_dijkstra_cm.awsm \
	dist/mi_fft.awsm \
	dist/mi_mandelbrot.awsm \
	dist/mi_mandelbrot_cm.awsm \
	dist/mi_qsort.awsm \
	dist/mi_qsort_cm.awsm \
	dist/mi_rsynth.awsm \
	dist/mi_sha.awsm \
	dist/mi_stringsearch.awsm \
	dist/mi_susan.awsm \
	dist/pb_datamining_correlation.awsm \
	dist/pb_datamining_covariance.awsm \
	dist/pb_la_blas_gemm.awsm \
	dist/pb_la_blas_gemver.awsm \
	dist/pb_la_blas_gesummv.awsm \
	dist/pb_la_blas_symm.awsm \
	dist/pb_la_blas_syr2k.awsm \
	dist/pb_la_blas_syrk.awsm \
	dist/pb_la_blas_trmm.awsm \
	dist/pb_la_kernels_2mm.awsm \
	dist/pb_la_kernels_3mm.awsm \
	dist/pb_la_kernels_atax.awsm \
	dist/pb_la_kernels_bicg.awsm \
	dist/pb_la_kernels_doitgen.awsm \
	dist/pb_la_kernels_mvt.awsm \
	dist/pb_la_solvers_cholesky.awsm \
	dist/pb_la_solvers_durbin.awsm \
	dist/pb_la_solvers_gramschmidt.awsm \
	dist/pb_la_solvers_lu.awsm \
	dist/pb_la_solvers_ludcmp.awsm \
	dist/pb_la_solvers_trisolv.awsm \
	dist/pb_medely_deriche.awsm \
	dist/pb_medely_floyd_warshall.awsm \
	dist/pb_medely_nussinov.awsm \
	dist/pb_stencils_adi.awsm \
	dist/pb_stencils_fdtd_2d.awsm \
	dist/pb_stencils_heat_3d.awsm \
	dist/pb_stencils_jacobi_1d.awsm \
	dist/pb_stencils_jacobi_2d.awsm \
	dist/pb_stencils_seidel_2d.awsm \
	dist/resize_image.awsm \
	dist/trap_divzero.awsm \
	dist/stack_overflow.awsm \
	dist/license_plate_detection.awsm \
	dist/gps_ekf.awsm \
	# dist/app_nn.out \
	# dist/custom_libjpeg.awsm \
	# dist/custom_sqlite.awsm \
	# dist/mi_gsm.awsm \
	# dist/mi_patricia.awsm \
	# dist/mi_patricia_cm.awsm \
	# dist/mi_pgp.awsm \

clean.bench:
	rm -rf bench

bench:
	mkdir -p bench

bench/app_tinycrypt.awsm.csv: dist/app_tinycrypt.awsm bench
	hyperfine -N -w 1 -r 2 --export-csv $@ \
		-n $(subst dist/,,$<) './$<'

bench/app_v9.awsm.csv: dist/app_v9.awsm bench
	hyperfine -N -w 1 -r 2 --export-csv $@ \
		-n $(subst dist/,,$<) './$<'

bench/CMSIS_5_NN.awsm.csv: dist/cifar10.awsm bench
	hyperfine -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) './$< < wasm_apps/CMSIS_5_NN/images/bmp/truck1.bmp'

bench/custom_binarytrees.awsm.csv: dist/custom_binarytrees.awsm bench
	hyperfine -N -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) './$< 16'

bench/custom_function_pointers.awsm.csv: dist/custom_function_pointers.awsm bench
	hyperfine -N -w 1 -r 2 --export-csv $@ \
		-n $(subst dist/,,$<) './$<'

bench/echo.awsm.csv: dist/echo.awsm bench
	hyperfine -N -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) 'echo "hi" | ./$<'

bench/fibonacci.awsm.csv: dist/fibonacci.awsm bench
	hyperfine -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) 'echo "40" | ./$<'

bench/gocr.awsm.csv: dist/gocr.awsm bench
	hyperfine -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) './$< < wasm_apps/gocr/examples/5x8.pnm'

bench/mi_adpcm.awsm.csv: dist/mi_adpcm.awsm bench
	hyperfine -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) './$< < wasm_apps/mi_adpcm/large.pcm'

bench/mi_bitcount.awsm.csv: dist/mi_bitcount.awsm bench
	hyperfine -N -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) './$< 16777216'

# TODO: Configure Preopens for filesystem
# bench/mi_crc.awsm.csv: dist/mi_crc.awsm bench
# 	hyperfine -N -w 10 --export-csv $@ \
# 		-n mi_crc_awsm './dist/mi_crc.awsm wasm_apps/mi_crc/large.pcm'

# TODO: Configure Preopens for filesystem
# bench/mi_dijkstra.awsm.csv: dist/mi_dijkstra.awsm bench
# 	hyperfine -N -w 10 --export-csv bench.csv \
# 		-n $(subst dist/,,$<) './$< ${ARGS}'

bench/mi_fft.awsm.csv: dist/mi_fft.awsm bench
	hyperfine -N -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) './dist/mi_fft.awsm 8 32768'

bench/mi_mandelbrot.awsm.csv: dist/mi_mandelbrot.awsm bench
	hyperfine -N -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) './$< 5000'

bench/mi_mandelbrot_cm.awsm.csv: dist/mi_mandelbrot_cm.awsm bench
	hyperfine -N -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) './$< 5000'


# TODO: Configure Preopens for filesystem
# bench/mi_qsort.awsm.csv: dist/mi_qsort.awsm bench
#	hyperfine -N -w 10 --export-csv $@ \
#		-n mi_qsort_awsm './dist/mi_qsort.awsm input_small.dat'

bench/mi_rsynth.awsm.csv: dist/mi_rsynth.awsm bench
	hyperfine -N -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) './$< -a -q -o /dev/null < wasm_apps/mi_rsynth/largeinput.txt'

# TODO: Configure Preopens for filesystem
# bench/mi_sha.awsm.csv: bench dist/mi_sha.awsm
# 	hyperfine -N -w 10 --export-csv $@ \
# 		-n mi_sha_awsm './dist/mi_sha.awsm wasm_apps/mi_sha/input_large.asc'

# TODO: Configure Preopens for filesystem
# bench/mi_susan.awsm.csv: bench dist/mi_susan.awsm

bench/license_plate_detection.awsm.csv: dist/license_plate_detection.awsm bench
	hyperfine -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) './$< <wasm_apps/sod/samples/plate.jpg'

bench/resize_image.awsm.csv: dist/resize_image.awsm bench
	hyperfine -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) './$< <wasm_apps/sod/samples/plate.jpg'

bench/traps.awsm.csv: bench dist/traps.awsm
	hyperfine -N -w 10 --export-csv $@ \
		-n $(subst dist/,,$<) 'echo "0" | ./$<'

bench/%.awsm.csv: dist/%.awsm bench
	hyperfine -N -w 10 --export-csv $@ -n $(subst dist/,,$<) './$<'

bench/%.csv: bench/%.awsm.csv
	make -C wasm_apps $@
	cat wasm_apps/$@ > $@
	tail -n +2 bench/$*.awsm.csv >> $@
	rm bench/$*.awsm.csv

BENCHMARKS= \
	bench/app_pid.csv \
	bench/app_tinycrypt.csv \
	bench/app_v9.csv \
	bench/CMSIS_5_NN.csv \
	bench/custom_binarytrees.csv \
	bench/custom_function_pointers.csv \
	bench/custom_matrix_multiply.csv \
	bench/custom_memcmp.csv \
	bench/echo.csv \
	bench/empty.csv \
	bench/exit.csv \
	bench/fibonacci.csv \
	bench/gocr.csv \
	bench/mi_adpcm.csv \
	bench/mi_basic_math.csv \
	bench/mi_bitcount.csv \
	bench/mi_bitcount_cm.csv \
	bench/mi_dijkstra_cm.csv \
	bench/mi_fft.csv \
	bench/mi_mandelbrot.csv \
	bench/mi_mandelbrot_cm.csv \
	bench/mi_qsort_cm.csv \
	bench/mi_rsynth.csv \
	bench/mi_stringsearch.csv \
	bench/pb_datamining_correlation.csv \
	bench/pb_datamining_covariance.csv \
	bench/pb_la_blas_gemm.csv \
	bench/pb_la_blas_gemver.csv \
	bench/pb_la_blas_gesummv.csv \
	bench/pb_la_blas_symm.csv \
	bench/pb_la_blas_syr2k.csv \
	bench/pb_la_blas_syrk.csv \
	bench/pb_la_blas_trmm.csv \
	bench/pb_la_kernels_2mm.csv \
	bench/pb_la_kernels_3mm.csv \
	bench/pb_la_kernels_atax.csv \
	bench/pb_la_kernels_bicg.csv \
	bench/pb_la_kernels_doitgen.csv \
	bench/pb_la_kernels_mvt.csv \
	bench/pb_la_solvers_cholesky.csv \
	bench/pb_la_solvers_durbin.csv \
	bench/pb_la_solvers_gramschmidt.csv \
	bench/pb_la_solvers_lu.csv \
	bench/pb_la_solvers_ludcmp.csv \
	bench/pb_la_solvers_trisolv.csv \
	bench/pb_medely_deriche.csv \
	bench/pb_medely_floyd_warshall.csv \
	bench/pb_medely_nussinov.csv \
	bench/pb_stencils_adi.csv \
	bench/pb_stencils_fdtd_2d.csv \
	bench/pb_stencils_heat_3d.csv \
	bench/pb_stencils_jacobi_1d.csv \
	bench/pb_stencils_jacobi_2d.csv \
	bench/license_plate_detection.csv \
	bench/resize_image.csv \
	bench/traps.csv

# Disabled due to various reasons
# bench/mi_crc.csv \
# bench/mi_dijkstra.csv \
# bench/mi_qsort.csv \
# bench/mi_sha.csv \
# bench/mi_susan.csv \

bench/bench.csv: ${BENCHMARKS}
	echo "command,mean,stddev,median,user,system,min,max" > $@
	tail -n +2 bench/$*.awsm.csv >> $@
	cat $^ | grep -v command,* >> $@bench/bench.csv
