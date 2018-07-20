from collections import namedtuple
import os
import subprocess as sp
import timeit

BENCH_ROOT = os.getcwd()
ROOT_PATH = os.path.dirname(BENCH_ROOT)
RUNTIME_PATH = ROOT_PATH + "/runtime"
SILVERFISH_PATH = ROOT_PATH + "/target/release/silverfish"
WASMCEPTION_PATH = ROOT_PATH + "/wasmception"

WASM_CLANG = WASMCEPTION_PATH + "/dist/bin/clang"
WASM_LINKER_FLAGS = "-Wl,--allow-undefined,-z,stack-size=40000,--no-threads,--stack-first,--no-entry,--export-all,--export=main,--export=dummy"
WASM_SYSROOT_FLAGS = "--sysroot={}/sysroot".format(WASMCEPTION_PATH)
WASM_FLAGS = WASM_LINKER_FLAGS + " --target=wasm32-unknown-unknown-wasm -nostartfiles -O3 -flto " + WASM_SYSROOT_FLAGS

RUN_COUNT = 10

Program = namedtuple("Program", ["name", "parameters"])

# These are the programs we're going to test with
programs = [
    Program("binarytrees", [18]),
    Program("fft", [5000, 2 ** 15]),
    Program("function_pointers", []),
    Program("mandelbrot", [7500]),
    Program("matrix_multiply", [])
]


# Now some helper methods for compiling code
def compile_to_executable(program):
    sp.check_call("clang -O3 -flto *.c -o bin/{}".format(program.name), shell=True, cwd=program.name)


def compile_to_wasm(program):
    command = "{clang} {flags} ../dummy.c *.c -o bin/{pname}.wasm".format(clang=WASM_CLANG, flags=WASM_FLAGS, pname=program.name)
    sp.check_call(command, shell=True, cwd=program.name)


def compile_wasm_to_bc(program):
    command = "{silverfish} bin/{pname}.wasm -o bin/{pname}.bc".format(silverfish=SILVERFISH_PATH, pname=program.name)
    sp.check_call(command, shell=True, cwd=program.name)


def compile_wasm_to_executable(program, exe_postfix, memory_impl):
    command = "clang -O3 -flto bin/{pname}.bc {runtime}/runtime.c {runtime}/libc/libc_backing.c {runtime}/memory/{mem_impl} -o bin/{pname}_{postfix}"\
        .format(pname=program.name, runtime=RUNTIME_PATH, mem_impl=memory_impl, postfix=exe_postfix)
    sp.check_call(command, shell=True, cwd=program.name)


def execute(p, args):
    command = p + " " + args
    sp.check_call(command, shell=True, stdout=sp.DEVNULL, stderr=sp.DEVNULL)


def bench(p, exe_postfix, name):
    print("Testing {} {}".format(p, name))
    path = "{broot}/{pname}/bin/{pname}{pf}".format(broot=BENCH_ROOT, pname=p.name, pf=exe_postfix)
    command = "execute('{path}', '{args}')".format(path=path, args=' '.join(map(str, p.parameters)))
    print(min(timeit.repeat(command, 'from __main__ import execute', number=1, repeat=RUN_COUNT)))


for p in programs:
    os.makedirs(p.name + "/bin", exist_ok=True)
    compile_to_executable(p)
    compile_to_wasm(p)
    compile_wasm_to_bc(p)
    compile_wasm_to_executable(p, "bc", "generic.c")
    compile_wasm_to_executable(p, "np", "no_protection.c")
    compile_wasm_to_executable(p, "vm", "64bit_nix.c")

for p in programs:
    bench(p, "", "native")
    bench(p, "_np", "no protection")
    bench(p, "_bc", "bounds checked")
    bench(p, "_vm", "virtual memory")
    print("")

