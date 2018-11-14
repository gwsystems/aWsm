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
ENABLE_DEBUG_SYMBOLS = False


# FIXME: Mibench runs many of these programs multiple times, which is probably worth replicating
class Program(object):
    def __init__(self, name, parameters, stack_size, custom_arguments=None, do_lto=True):
        self.name = name
        self.parameters = parameters
        self.stack_size = stack_size
        self.custom_arguments = ""
        if custom_arguments:
            self.custom_arguments = " ".join(custom_arguments)
        self.do_lto = do_lto

    def __str__(self):
        return "{}({})".format(self.name, " ".join(map(str, self.parameters)))


# These are the programs we're going to test with
# TODO: Fix ispell, which doesn't compile on OS X
# TODO: Fix sphinx, which doesn't work properly on OS X
# TODO: Do ghostscript, which has a ton of files
programs = [
    Program("adpcm", ["< large.pcm"], 2 ** 14,
            custom_arguments=["-Wno-implicit-int", "-Wno-implicit-function-declaration"]),
    Program("basic_math", [], 2 ** 14),
    Program("binarytrees", [16], 2 ** 14),
    Program("bitcount", [2 ** 24], 2 ** 14),
    Program("crc", ["large.pcm"], 2 ** 14, custom_arguments=["-Wno-implicit-int", "-Wno-format"]),
    Program("dijkstra", ["input.dat"], 2 ** 14,
            custom_arguments=["-Wno-return-type"]),
    Program("fft", [8, 32768], 2 ** 14),
    Program("function_pointers", [], 2 ** 14),
    Program("gsm", ["-fps", "-c", "large.au"], 2 ** 15, custom_arguments=["-DSASR", "-Wno-everything"]),
    Program("mandelbrot", [5000], 2 ** 14),
    Program("matrix_multiply", [], 2 ** 14),
    Program("patricia", ["large.udp"], 2 ** 14),
    Program("qsort", ["input_small.dat"], 2 ** 18),
    Program("rsynth", ["-a", "-q", "-o", "/dev/null", "< largeinput.txt"], 2**14,
            custom_arguments=["-I.", "-Wno-everything", "-I/usr/local/include/"]),
    Program("sha", ["input_large.asc"], 2 ** 14),
    Program("susan", ["input_large.pgm", "/dev/null", "-s"], 2 ** 19, custom_arguments=["-Wno-everything"]),
    Program("stringsearch", [], 2 ** 13),


    # TODO: These programs segfault on my computer...
    # Program("blowfish", ["e", "input_large.asc", "/dev/null", "1234567890abcdeffedcba0987654321"], 2**14),
    # Program("pgp", ['-sa -z "this is a test" -u taustin@eecs.umich.edu testin.txt austin@umich.edu'], 2 ** 14,
    #         custom_arguments=["-DUNIX -D_BSD -DPORTABLE -DUSE_NBIO -DMPORTABLE", "-I.", "-Wno-everything"]),

]


# Now some helper methods for compiling code
def compile_to_executable(program):
    opt = "-O3"
    if program.do_lto:
        opt += " -flto"
    if ENABLE_DEBUG_SYMBOLS:
        opt += " -g"
    sp.check_call("clang {} -lm {} *.c -o bin/{}".format(program.custom_arguments, opt, program.name), shell=True, cwd=program.name)


def compile_to_wasm(program):
    flags = WASM_FLAGS.format(stack_size=program.stack_size)
    command = "{clang} {flags} {args} -O3 -flto ../dummy.c *.c -o bin/{pname}.wasm" \
        .format(clang=WASM_CLANG, flags=flags, args=program.custom_arguments, pname=program.name)
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
    opt = "-O3"
    if program.do_lto:
        opt += " -flto"
    if ENABLE_DEBUG_SYMBOLS:
        opt += " -g"
    command = "clang -lm {opt} {bc_file} {runtime}/runtime.c {runtime}/libc/libc_backing.c {runtime}/memory/{mem_impl} -o bin/{pname}_{postfix}"\
        .format(opt=opt, bc_file=bc_file, pname=program.name, runtime=RUNTIME_PATH, mem_impl=memory_impl, postfix=exe_postfix)
    sp.check_call(command, shell=True, cwd=program.name)


def execute(p, args, dir):
    command = p + " " + args
    sp.check_call(command, shell=True, stdout=sp.DEVNULL, stderr=sp.DEVNULL, cwd=dir)


def bench(p, exe_postfix, name):
    print("Testing {} {}".format(p, name))
    path = "{broot}/{pname}/bin/{pname}{pf}".format(broot=BENCH_ROOT, pname=p.name, pf=exe_postfix)
    command = "execute('{path}', '{args}', '{dir}')".format(path=path, args=' '.join(map(str, p.parameters)), dir=p.name)
    return min(timeit.repeat(command, 'from __main__ import execute', number=1, repeat=RUN_COUNT))


def output_run(base_time, execution_time):
    base_time = round(base_time, 4)
    execution_time = round(execution_time, 4)
    if execution_time > base_time:
        print("{:.4f} ({:.2f}% slower)".format(execution_time, ((execution_time - base_time) / base_time) * 100))
    else:
        print("{:.4f} ({:.2f}% faster)".format(execution_time, ((base_time - execution_time) / base_time) * 100))


for i, p in enumerate(programs):
    print("Compiling {} {}/{}".format(p.name, i + 1, len(programs)))

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

print()

for p in programs:
    base_speed = bench(p, "", "native")
    print("{:.4f}".format(base_speed))

    np_speed = bench(p, "_np", "wasm no protection")
    output_run(base_speed, np_speed)

    bc_speed = bench(p, "_bc", "wasm bounds checked")
    output_run(base_speed, bc_speed)

    if IS_X86:
        mpx_speed = bench(p, "_mpx", "wasm using mpx")
        output_run(base_speed, mpx_speed)
    if IS_64_BIT:
        vm_speed = bench(p, "_vm", "wasm virtual memory")
        output_run(base_speed, vm_speed)
    if IS_32_BIT_X86:
        sm_speed = bench(p, "_sm", "wasm using segmentation")
        output_run(base_speed, sm_speed)
    print("")

