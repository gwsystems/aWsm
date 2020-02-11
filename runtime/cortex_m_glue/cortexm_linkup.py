import subprocess as sp

# NOTE: Known good values from experimentation with Joey
# vagrant@ubuntu-xenial:~/silverfish/runtime/cortex_m_glue$ clang-9 -std=c11 -ffreestanding -falign-functions=16 -target thumbv7m-none-unknown-eabi -mcpu=cortex-m7 -mfloat-abi=softfp -mthumb -munaligned-access -flto -g -O3 -nostdlib -fno-builtin -c ../runtime.c ../memory/cortex_m_wrapping.c ../libc/cortex_m_backing.c printf.c ../../code_benches/pb_la_kernels_atax/bin/pb_la_kernels_atax.bc
# warning: overriding the module target triple with thumbv7em-none-unknown-eabi [-Woverride-module]
#     1 warning generated.
# vagrant@ubuntu-xenial:~/silverfish/runtime/cortex_m_glue$ clang-9 -std=c11 -ffreestanding -falign-functions=16 -target thumbv7m-none-unknown-eabi -mcpu=cortex-m7 -mfloat-abi=softfp -mthumb -munaligned-access -flto -g -O3 -nostdlib -fno-builtin -r *.o -o out/pb_la_kernels_atax_main.o

# TODO: `-static`?
CLANG = "clang-9"
GCC = "arm-none-eabi-gcc"

SYS_FLAGS = "-std=c11 -nostdlib -fno-builtin -ffreestanding -falign-functions=16"
TARGET_FLAGS = "-mcpu=cortex-m3 -mfloat-abi=soft -mthumb"
# TARGET_FLAGS = "-mcpu=cortex-m7 -mfloat-abi=softfp -mthumb -munaligned-access"
CC_TARGET = "-target thumbv7m-none-unknown-eabi"
OPT = "-g -O0"
# OPT = "-g -O3 -flto"

RUNTIME_FILES = "../runtime.c ../memory/cortex_m_wrapping.c ../libc/cortex_m_backing.c printf.c"

START_FILES = "start.s qemu_putchar.c"
RT = "~/m3/compiler-rt/lib/baremetal/libclang_rt.builtins-arm.a"
# RT = "~/m7/compiler-rt/lib/baremetal/libclang_rt.builtins-arm.a"

CLANG_LD = CLANG + " " + CC_TARGET


programs = [
    "mi_basic_math",
    "mi_bitcount_cm",
    "mi_qsort_cm",
    "mi_fft_cm",
    "mi_stringsearch",

    # "pb_datamining_correlation",
    # "pb_datamining_covariance",
    #
    # "pb_la_blas_gemm",
    # "pb_la_blas_gemver",
    # "pb_la_blas_gesummv",
    # "pb_la_blas_symm",
    # "pb_la_blas_syr2k",
    # "pb_la_blas_syrk",
    # "pb_la_blas_trmm",
    #
    # "pb_la_kernels_2mm",
    # "pb_la_kernels_3mm",
    # "pb_la_kernels_atax",
    # "pb_la_kernels_bicg",
    # "pb_la_kernels_doitgen",
    # "pb_la_kernels_mvt",
    #
    # "pb_la_solvers_cholesky",
    # "pb_la_solvers_durbin",
    # "pb_la_solvers_gramschmidt",
    # "pb_la_solvers_lu",
    # "pb_la_solvers_ludcmp",
    # "pb_la_solvers_trisolv",
    #
    # "pb_medely_deriche",
    # "pb_medely_floyd_warshall",
    # "pb_medely_nussinov",
    #
    # "pb_stencils_adi",
    # "pb_stencils_fdtd_2d",
    # "pb_stencils_heat_3d",
    # "pb_stencils_jacobi_1d",
    # "pb_stencils_jacobi_2d",
    # "pb_stencils_seidel_2d",
]

for pname in programs:
    print("======", pname, "======")

    linkup_bc = "../../code_benches/{pname}/bin/{pname}.bc".format(pname=pname)
    output_o = "out/{}_baremetal.o".format(pname)
    output_elf = "out/{}_qemu_rdy.elf".format(pname)

    compile_bc_and_runtime = "{cc} {sys_flags} {cc_target} {target_flags} {opt} -c {runtime_files} {bc}"\
        .format(cc=CLANG, sys_flags=SYS_FLAGS, cc_target=CC_TARGET, target_flags=TARGET_FLAGS,
                opt=OPT, runtime_files=RUNTIME_FILES, bc=linkup_bc)

    link_o = "{ld} {sys_flags} {target_flags} {opt} -r -o {obj} *.o" \
        .format(ld=CLANG_LD, sys_flags=SYS_FLAGS, target_flags=TARGET_FLAGS, opt=OPT, obj=output_o)

    link_elf = "{ld} {sys_flags} {target_flags} {opt} -o {elf} {start} {obj} {rt}" \
        .format(ld=CLANG_LD, sys_flags=SYS_FLAGS, target_flags=TARGET_FLAGS, opt=OPT.replace("-flto", ""),
                elf=output_elf, start=START_FILES, obj=output_o, rt=RT)

    # Setup and cleanup
    sp.check_call("mkdir -p out", shell=True)
    sp.check_call("rm -f *.o " + output_elf, shell=True)

    # Compile the bytecode + runtime, then make a baremetal .o
    print(compile_bc_and_runtime)
    sp.check_call(compile_bc_and_runtime, shell=True)
    print(link_o)
    sp.check_call(link_o, shell=True)

    # Then add in the start files and make an elf
    print(link_elf)
    sp.check_call(link_elf, shell=True)

    sp.check_call("rm *.o", shell=True)
    print()
