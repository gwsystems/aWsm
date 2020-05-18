import subprocess as sp

# NOTE: Known good values from experimentation with Joey
# vagrant@ubuntu-xenial:~/silverfish/runtime/cortex_m_glue$ clang-9 -std=c11 -ffreestanding -falign-functions=16 -target thumbv7m-none-unknown-eabi -mcpu=cortex-m7 -mfloat-abi=softfp -mthumb -munaligned-access -flto -g -O3 -nostdlib -fno-builtin -c ../runtime.c ../memory/cortex_m_wrapping.c ../libc/cortex_m_backing.c printf.c ../../code_benches/pb_la_kernels_atax/bin/pb_la_kernels_atax.bc
# warning: overriding the module target triple with thumbv7em-none-unknown-eabi [-Woverride-module]
#     1 warning generated.
# vagrant@ubuntu-xenial:~/silverfish/runtime/cortex_m_glue$ clang-9 -std=c11 -ffreestanding -falign-functions=16 -target thumbv7m-none-unknown-eabi -mcpu=cortex-m7 -mfloat-abi=softfp -mthumb -munaligned-access -flto -g -O3 -nostdlib -fno-builtin -r *.o -o out/pb_la_kernels_atax_main.o

M3_DEBUG = False

CLANG = "clang-9"
CLANG_PP = "clang++-9"
GCC = "arm-none-eabi-gcc"
LLC = "llc-9"
OPT_CMD = "opt-9"

SYS_FLAGS = "-nostdlib -nostdinc -static -ffreestanding -falign-functions=16 -fomit-frame-pointer"
# SYS_FLAGS = "-nostdlib -nostdinc -static -ffreestanding -falign-functions=16"

if M3_DEBUG:
    TARGET_FLAGS = "-mcpu=cortex-m3 -mfloat-abi=soft -mthumb"
else:
    TARGET_FLAGS = "-mcpu=cortex-m7 -mfloat-abi=softfp -mthumb -munaligned-access"
    # TARGET_FLAGS = "-mcpu=cortex-m7 -mfloat-abi=softfp -mthumb -mno-unaligned-access"

if M3_DEBUG:
    OPT = "-g -O0"
    # OPT = "-g -O3 -flto"
    CC_TARGET = "-target thumbv7m-none-unknown-eabi"
else:
    OPT = "-O3 -flto -g"
    # OPT = "-Os -flto -g"
    CC_TARGET = "-target thumbv7em-none-unknown-eabi"

RUNTIME_FILES = "../runtime.c ../libc/cortex_m_backing.c printf.c"

START_FILES = "start.s qemu_putchar.c qemu_pid.c"

if M3_DEBUG:
    LLC_FLAGS = ""
    RT = "~/m3/compiler-rt/lib/baremetal/libclang_rt.builtins-arm.a"
else:
    LLC_FLAGS = "-O3 -mattr=+armv7e-m,+dsp,+fp-armv8d16,+fp-armv8d16sp,+fp16,+fp64,+fpregs,+hwdiv,+thumb-mode,+vfp2,+vfp2d16,+vfp2d16sp,+vfp2sp,+vfp3d16,+vfp3d16sp,+vfp4d16,+vfp4d16sp,-aes,-crc,-crypto,-dotprod,-fp16fml,-fullfp16,-hwdiv-arm,-lob,-mve,-mve.fp,-ras,-sb,-sha2"
    RT = "~/m7/compiler-rt/lib/baremetal/libclang_rt.builtins-arm.a"

CLANG_LD = CLANG + " " + CC_TARGET

SYS_INC = "-isystem /usr/lib/gcc/arm-none-eabi/4.9.3/include -isystem /usr/lib/gcc/arm-none-eabi/4.9.3/include-fixed -isystem /usr/lib/arm-none-eabi/include"

programs = [
    "_test",
    "app_nn",
    "app_pid",
    "app_tiny_ekf",
    "app_tinycrypt",

    "mi_basic_math",
    "mi_bitcount_cm",
    "mi_dijkstra_cm",
    "mi_fft_cm",
    "mi_mandelbrot_cm",
    "mi_qsort_cm",
    "mi_stringsearch",

    "pb_datamining_correlation",
    "pb_datamining_covariance",

    "pb_la_blas_gemm",
    "pb_la_blas_gemver",
    "pb_la_blas_gesummv",
    "pb_la_blas_symm",
    "pb_la_blas_syr2k",
    "pb_la_blas_syrk",
    "pb_la_blas_trmm",

    "pb_la_kernels_2mm",
    "pb_la_kernels_3mm",
    "pb_la_kernels_atax",
    "pb_la_kernels_bicg",
    "pb_la_kernels_doitgen",
    "pb_la_kernels_mvt",

    "pb_la_solvers_cholesky",
    "pb_la_solvers_durbin",
    "pb_la_solvers_gramschmidt",
    "pb_la_solvers_lu",
    "pb_la_solvers_ludcmp",
    "pb_la_solvers_trisolv",

    "pb_medely_deriche",
    "pb_medely_floyd_warshall",
    "pb_medely_nussinov",

    "pb_stencils_adi",
    "pb_stencils_fdtd_2d",
    "pb_stencils_heat_3d",
    "pb_stencils_jacobi_1d",
    "pb_stencils_jacobi_2d",
    "pb_stencils_seidel_2d",
]

memory_backends = [
    ("no_protection", "../memory/cortex_m_no_protection.c"),
    ("bounds_checked", "../memory/cortex_m.c"),
    ("wrapping", "../memory/cortex_m_wrapping.c"),
    ("spt", "../memory/cortex_m_spt.c"),
]

print("there are", len(programs), "programs")

for pname in programs:
    for (mname, mpath) in memory_backends:
        print("====== {pname}({mname}) ======".format(pname=pname, mname=mname))

        original_bc = "../../code_benches/{pname}/bin/{pname}.bc".format(pname=pname)
        optimized_bc = "{pname}_opt.bc".format(pname=pname)
        output_o = "out/{}_bm_{}.o".format(pname, mname)
        output_elf = "out/{}_qemu_{}.elf".format(pname, mname)

        s_name = "{pname}.s".format(pname=pname)
        # compile_bc_to_s = "{llc} {llc_flags} -o {s_name} {bc}"\
        #     .format(llc=LLC, llc_flags=LLC_FLAGS, s_name=s_name, bc=linkup_bc)

        optimize_bc = "{opt} --O3 -o {opt_bc} {bc}".format(opt=OPT_CMD, opt_bc=optimized_bc, bc=original_bc)
        optimize_bc_again = "{opt} --O3 -o {bc} {bc}".format(opt=OPT_CMD, bc=optimized_bc)

        compile_runtime = "{cc} {sys_flags} {cc_target} {target_flags} {opt} {sys_inc} -c {runtime_files} {mem} {bc}"\
            .format(cc=CLANG, sys_flags=SYS_FLAGS, cc_target=CC_TARGET, target_flags=TARGET_FLAGS,
                    opt=OPT, sys_inc=SYS_INC, runtime_files=RUNTIME_FILES, mem=mpath, bc=optimized_bc)

        # link_o = "{ld} {sys_flags} {target_flags} {opt} -r -o {obj} *.o {s_name}" \
        #     .format(ld=CLANG_LD, sys_flags=SYS_FLAGS, target_flags=TARGET_FLAGS, opt=OPT, obj=output_o, s_name=s_name)
        link_o = "{ld} {sys_flags} {target_flags} {opt} -r -o {obj} *.o" \
            .format(ld=CLANG_LD, sys_flags=SYS_FLAGS, target_flags=TARGET_FLAGS, opt=OPT, obj=output_o)

        link_elf = "{ld} {sys_flags} {target_flags} {opt} -o {elf} {start} {obj} {rt}" \
            .format(ld=CLANG_LD, sys_flags=SYS_FLAGS, target_flags=TARGET_FLAGS, opt=OPT.replace("-flto", ""),
                    elf=output_elf, start=START_FILES, obj=output_o, rt=RT)

        # Setup and cleanup
        sp.check_call("mkdir -p out", shell=True)
        sp.check_call("rm -f *.o *.bc {}".format(s_name), shell=True)

        # Compile the bytecode + runtime, then make a baremetal .o
        # print(compile_bc_to_s)
        # sp.check_call(compile_bc_to_s, shell=True)
        print(optimize_bc)
        sp.check_call(optimize_bc, shell=True)
        for _ in range(3):
            print(optimize_bc_again)
            sp.check_call(optimize_bc_again, shell=True)
        print(compile_runtime)
        sp.check_call(compile_runtime, shell=True)
        print(link_o)
        sp.check_call(link_o, shell=True)

        # Then add in the start files and make an elf
        # print(link_elf)
        # sp.check_call(link_elf, shell=True)

        sp.check_call("rm -f *.o *.bc {}".format(s_name), shell=True)
        print()

    print("====== {pname}(c) ======".format(pname=pname))
    output_o = "out/{}_bm_{}.o".format(pname, "c")
    output_elf = "out/{}_qemu_{}.elf".format(pname, "c")

    if pname == "app_tiny_ekf" or pname == "app_pid":
        c_files = "../../code_benches/{pname}/*.cpp".format(pname=pname)

        compile_c = "{cppc}  -fno-rtti -fno-exceptions {sys_flags} {cc_target} {target_flags} {opt} {sys_inc} -c {c}; {cc} {sys_flags} {cc_target} {target_flags} {opt} {sys_inc} -c math*.c" \
            .format(cc=CLANG, cppc=CLANG_PP, sys_flags=SYS_FLAGS, cc_target=CC_TARGET, target_flags=TARGET_FLAGS,
                    opt=OPT, sys_inc=SYS_INC, c=c_files)
    else:
        c_files = "../../code_benches/{pname}/*.c".format(pname=pname)

        compile_c = "{cc} {sys_flags} {cc_target} {target_flags} {opt} -I/home/vagrant/silverfish/code_benches/app_ekf/ -I/home/vagrant/silverfish/code_benches/app_tinycrypt/ -DENABLE_TESTS -DARM_MATH_CM3 -I/home/vagrant/CMSIS_5_NN/CMSIS_5/CMSIS/DSP/Include -I/home/vagrant/CMSIS_5_NN/CMSIS_5/CMSIS/Core/Include -I/home/vagrant/CMSIS_5_NN/CMSIS_5/CMSIS/NN/Include {sys_inc} -c {c} math*.c"\
            .format(cc=CLANG, sys_flags=SYS_FLAGS, cc_target=CC_TARGET, target_flags=TARGET_FLAGS,
                    opt=OPT, sys_inc=SYS_INC, c=c_files)
    link_o = "{ld} {sys_flags} {target_flags} {opt} -r -o {obj} *.o" \
        .format(ld=CLANG_LD, sys_flags=SYS_FLAGS, target_flags=TARGET_FLAGS, opt=OPT, obj=output_o)
    rename_o = "llvm-objcopy-9 --prefix-symbols pb_ {obj}".format(obj=output_o)

    # link_elf = "{ld} {sys_flags} {target_flags} {opt} -o {elf} {start} {obj} {rt}" \
    #     .format(ld=CLANG_LD, sys_flags=SYS_FLAGS, target_flags=TARGET_FLAGS, opt=OPT.replace("-flto", ""),
    #             elf=output_elf, start=START_FILES, obj=output_o, rt=RT)

    # Setup and cleanup
    sp.check_call("mkdir -p out", shell=True)
    sp.check_call("rm -f *.o", shell=True)

    # Compile the bytecode + runtime, then make a baremetal .o
    print(compile_c)
    sp.check_call(compile_c, shell=True)
    print(link_o)
    sp.check_call(link_o, shell=True)
    print(rename_o)
    sp.check_call(rename_o, shell=True)
    # print(link_elf)
    # sp.check_call(link_elf, shell=True)

    sp.check_call("rm *.o", shell=True)
    print()


