from collections import namedtuple
import os
import subprocess as sp
import sys
import timeit

BENCH_ROOT = os.getcwd()
ROOT_PATH = os.path.dirname(BENCH_ROOT)
RUNTIME_PATH = ROOT_PATH + "/runtime"
SILVERFISH_PATH = ROOT_PATH + "/target/release/silverfish"
WASMCEPTION_PATH = ROOT_PATH + "/wasmception"

WASM_CLANG = WASMCEPTION_PATH + "/dist/bin/clang"
WASM_LINKER_FLAGS = "-Wl,--allow-undefined,-z,stack-size={stack_size},--no-threads,--stack-first,--no-entry,--export-all,--export=main,--export=dummy"
WASM_SYSROOT_FLAGS = "--sysroot={}/sysroot".format(WASMCEPTION_PATH)
WASM_FLAGS = WASM_LINKER_FLAGS + " --target=wasm32-unknown-unknown-wasm -nostartfiles -O3 -flto " + WASM_SYSROOT_FLAGS

IS_64_BIT = sys.maxsize > 2**32
IS_X86 = '86' in os.uname()[-1]
IS_32_BIT_X86 = (not IS_64_BIT) and IS_X86

RUN_COUNT = 10


Program = namedtuple("Program", ["name", "parameters", "stack_size"])

# These are the programs we're going to test with
programs = [
    Program("basic_math", [], 2 ** 14),
    Program("binarytrees", [18], 2 ** 14),
    Program("bitcount", [2 ** 24], 2 ** 14),
    Program("fft", [5000, 2 ** 15], 2 ** 14),
    Program("function_pointers", [], 2 ** 14),
    Program("mandelbrot", [7500], 2 ** 14),
    Program("matrix_multiply", [], 2 ** 14),
    Program("patricia", ["large.udp"], 2 ** 14),
    Program("qsort", ["input_small.dat"], 2 ** 18),
    Program("sha", ["input_large.asc"], 2 ** 14),
    Program("stringsearch", [], 2 ** 13),
]


# Now some helper methods for compiling code
def compile_to_executable(program):
    sp.check_call("clang -g -O3 -flto -lm *.c -o bin/{}".format(program.name), shell=True, cwd=program.name)


def compile_to_wasm(program):
    flags = WASM_FLAGS.format(stack_size=program.stack_size)
    command = "{clang} {flags} ../dummy.c *.c -o bin/{pname}.wasm".format(clang=WASM_CLANG, flags=flags, pname=program.name)
    sp.check_call(command, shell=True, cwd=program.name)


def compile_wasm_to_bc(program):
    command = "{silverfish} bin/{pname}.wasm -o bin/{pname}.bc".format(silverfish=SILVERFISH_PATH, pname=program.name)
    sp.check_call(command, shell=True, cwd=program.name)
    # Compile a second version with runtime globlas
    command = "{silverfish} -i --runtime_globals bin/{pname}.wasm -o bin/{pname}_rg.bc".format(silverfish=SILVERFISH_PATH, pname=program.name)
    sp.check_call(command, shell=True, cwd=program.name)



def compile_wasm_to_executable(program, exe_postfix, memory_impl, runtime_globals=False):
    bc_file = "bin/{pname}.bc".format(pname=program.name)
    if runtime_globals:
        bc_file = "bin/{pname}_rg.bc".format(pname=program.name)
    command = "clang -g -lm -O3 -flto {bc_file} {runtime}/runtime.c {runtime}/libc/libc_backing.c {runtime}/memory/{mem_impl} -o bin/{pname}_{postfix}"\
        .format(bc_file=bc_file, pname=program.name, runtime=RUNTIME_PATH, mem_impl=memory_impl, postfix=exe_postfix)
    sp.check_call(command, shell=True, cwd=program.name)


def execute(p, args, dir):
    command = p + " " + args
    sp.check_call(command, shell=True, stdout=sp.DEVNULL, stderr=sp.DEVNULL, cwd=dir)


def bench(p, exe_postfix, name):
    print("Testing {} {}".format(p, name))
    path = "{broot}/{pname}/bin/{pname}{pf}".format(broot=BENCH_ROOT, pname=p.name, pf=exe_postfix)
    command = "execute('{path}', '{args}', '{dir}')".format(path=path, args=' '.join(map(str, p.parameters)), dir=p.name)
    print(min(timeit.repeat(command, 'from __main__ import execute', number=1, repeat=RUN_COUNT)))


for p in programs:
    os.makedirs(p.name + "/bin", exist_ok=True)
    compile_to_executable(p)
    compile_to_wasm(p)
    compile_wasm_to_bc(p)
    compile_wasm_to_executable(p, "bc", "generic.c")
    compile_wasm_to_executable(p, "np", "no_protection.c")
    if IS_64_BIT:
        compile_wasm_to_executable(p, "vm", "64bit_nix.c")
    if IS_X86:
        compile_wasm_to_executable(p, "mpx", "mpx.c")
    if IS_32_BIT_X86:
        compile_wasm_to_executable(p, "sm", "segmented.c")


for p in programs:
    bench(p, "", "native")
    bench(p, "_np", "wasm no protection")
    bench(p, "_bc", "wasm bounds checked")
    if IS_64_BIT:
        bench(p, "_vm", "wasm virtual memory")
    if IS_X86:
        bench(p, "_mpx", "wasm using mpx")
    if IS_32_BIT_X86:
        bench(p, "_sm", "wasm using segmentation")
    print("")

