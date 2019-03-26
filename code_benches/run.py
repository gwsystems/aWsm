import csv
import os
import subprocess as sp
import sys
import timeit

# CSV file name
CSV_NAME = "benchmarks.csv"

# Absolute path to the `code_benches` directory
BENCH_ROOT = os.getcwd()
# Absolute path to the `silverfish` directory
ROOT_PATH = os.path.dirname(BENCH_ROOT)

RUNTIME_PATH = ROOT_PATH + "/runtime"
SILVERFISH_PATH = ROOT_PATH + "/target/release/silverfish"
WASMCEPTION_PATH = ROOT_PATH + "/wasmception"

# Our special WASM clang is under this wasmception path
WASM_CLANG = WASMCEPTION_PATH + "/dist/bin/clang"
# These flags are all somewhat important -- see @Others for more information
WASM_LINKER_FLAGS = "-Wl,--allow-undefined,-z,stack-size={stack_size},--no-threads,--stack-first,--no-entry,--export-all,--export=main,--export=dummy"
# Point WASM to our custom libc
WASM_SYSROOT_FLAGS = "--sysroot={}/sysroot".format(WASMCEPTION_PATH)
WASM_FLAGS = WASM_LINKER_FLAGS + " --target=wasm32-unknown-unknown-wasm -nostartfiles -O3 -flto " + WASM_SYSROOT_FLAGS

# What is the machine we're running on like?
IS_64_BIT = sys.maxsize > 2**32
IS_X86 = '86' in os.uname()[-1]
IS_32_BIT_X86 = (not IS_64_BIT) and IS_X86

# TODO: Add an option to output a CSV instead of human readable output

# How many times should we run our benchmarks
RUN_COUNT = 10
ENABLE_DEBUG_SYMBOLS = True


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
    # Real world program benchmarks
    Program("libjpeg", [], 2 ** 15,
            custom_arguments=["-Wno-incompatible-library-redeclaration", "-Wno-implicit-function-declaration", "-Wno-shift-negative-value"]),
    Program("sqlite", [], 2 ** 15),

    # Synthetic benchmarks
    Program("binarytrees", [16], 2 ** 14),
    Program("function_pointers", [], 2 ** 14),
    Program("matrix_multiply", [], 2 ** 14),

    # Benchmark programs
    Program("adpcm", ["< large.pcm"], 2 ** 14,
            custom_arguments=["-Wno-implicit-int", "-Wno-implicit-function-declaration"]),
    Program("basic_math", [], 2 ** 14),
    Program("bitcount", [2 ** 24], 2 ** 14),
    Program("crc", ["large.pcm"], 2 ** 14, custom_arguments=["-Wno-implicit-int", "-Wno-format"]),
    Program("dijkstra", ["input.dat"], 2 ** 14,
            custom_arguments=["-Wno-return-type"]),
    Program("fft", [8, 32768], 2 ** 14),
    Program("gsm", ["-fps", "-c", "large.au"], 2 ** 15, custom_arguments=["-DSASR", "-Wno-everything"]),
    Program("mandelbrot", [5000], 2 ** 14),
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


# Compile the C code in `program`'s directory into a native executable
def compile_to_executable(program):
    opt = "-O3"
    if program.do_lto:
        opt += " -flto"
    if ENABLE_DEBUG_SYMBOLS:
        opt += " -g"
    sp.check_call("clang {} -lm {} *.c -o bin/{}".format(program.custom_arguments, opt, program.name), shell=True, cwd=program.name)


# Compile the C code in `program`'s directory into WASM
def compile_to_wasm(program):
    flags = WASM_FLAGS.format(stack_size=program.stack_size)
    command = "{clang} {flags} {args} -O3 -flto ../dummy.c *.c -o bin/{pname}.wasm" \
        .format(clang=WASM_CLANG, flags=flags, args=program.custom_arguments, pname=program.name)
    sp.check_call(command, shell=True, cwd=program.name)


# Compile the WASM in `program`'s directory into llvm bytecode
def compile_wasm_to_bc(program):
    command = "{silverfish} bin/{pname}.wasm -o bin/{pname}.bc".format(silverfish=SILVERFISH_PATH, pname=program.name)
    sp.check_call(command, shell=True, cwd=program.name)
    # Also compile an unsafe version, so we can see the performance difference
    command = "{silverfish} -u bin/{pname}.wasm -o bin/{pname}_us.bc".format(silverfish=SILVERFISH_PATH, pname=program.name)
    sp.check_call(command, shell=True, cwd=program.name)


# Compile the WASM in `program`'s directory into llvm bytecode
def compile_wasm_to_executable(program, exe_postfix, memory_impl, unsafe_impls=False):
    bc_file = "bin/{pname}.bc".format(pname=program.name)
    if unsafe_impls:
        bc_file = "bin/{pname}_us.bc".format(pname=program.name)
    opt = "-O3"
    if program.do_lto:
        opt += " -flto"
    if ENABLE_DEBUG_SYMBOLS:
        opt += " -g"
    command = "clang -lm {opt} {bc_file} {runtime}/runtime.c {runtime}/libc/libc_backing.c {runtime}/memory/{mem_impl} -o bin/{pname}_{postfix}"\
        .format(opt=opt, bc_file=bc_file, pname=program.name, runtime=RUNTIME_PATH, mem_impl=memory_impl, postfix=exe_postfix)
    sp.check_call(command, shell=True, cwd=program.name)


# Execute executable `p` with arguments `args` in directory `dir`
def execute(p, args, dir):
    command = p + " " + args
    sp.check_call(command, shell=True, stdout=sp.DEVNULL, stderr=sp.DEVNULL, cwd=dir)


# Benchmark the given program's executable
#   p = the program
#   exe_postfix = what postfix we gave the executable in `compile_wasm_to_executable`
#   name = the human readable name for this version of the executable
def bench(p, exe_postfix, name):
    print("Testing {} {}".format(p, name))
    path = "{broot}/{pname}/bin/{pname}{pf}".format(broot=BENCH_ROOT, pname=p.name, pf=exe_postfix)
    command = "execute('{path}', '{args}', '{dir}')".format(path=path, args=' '.join(map(str, p.parameters)), dir=p.name)
    return min(timeit.repeat(command, 'from __main__ import execute', number=1, repeat=RUN_COUNT))


# Output a run's execution time, telling us how much faster or slower it is
def output_run(base_time, execution_time):
    base_time = round(base_time, 4)
    execution_time = round(execution_time, 4)
    if execution_time > base_time:
        print("{:.4f} ({:.2f}% slower)".format(execution_time, ((execution_time - base_time) / base_time) * 100))
    else:
        print("{:.4f} ({:.2f}% faster)".format(execution_time, ((base_time - execution_time) / base_time) * 100))


# Compile all our programs
for i, p in enumerate(programs):
    print("Compiling {} {}/{}".format(p.name, i + 1, len(programs)))

    os.makedirs(p.name + "/bin", exist_ok=True)
    compile_to_executable(p)
    compile_to_wasm(p)
    compile_wasm_to_bc(p)
    compile_wasm_to_executable(p, "np_us", "no_protection.c", True)
    compile_wasm_to_executable(p, "np", "no_protection.c")
    compile_wasm_to_executable(p, "bc", "generic.c")
    if IS_64_BIT:
        compile_wasm_to_executable(p, "vm", "64bit_nix.c")
    if IS_X86:
        compile_wasm_to_executable(p, "mpx", "mpx.c")
    if IS_32_BIT_X86:
        compile_wasm_to_executable(p, "sm", "segmented.c")

print()
print("Outputting to " + CSV_NAME)
print()

with open(CSV_NAME, 'w+', newline='') as csv_file:
    csv_writer = csv.writer(csv_file)

    columns = ["Program", "native", "wasm no protection unsafe", "wasm no protection", "wasm bounds checked"]
    if IS_X86:
        columns.append("wasm mpx")
    if IS_64_BIT:
        columns.append("wasm virtual memory")
    if IS_32_BIT_X86:
        columns.append("wasm segmentation")
    csv_writer.writerow(columns)

    # Benchmark and output timing info for each of our programs
    for p in programs:
        csv_row = [p.name]

        base_speed = bench(p, "", "native")
        print("{:.4f}".format(base_speed))
        csv_row.append(base_speed / base_speed)

        np_u_speed = bench(p, "_np_us", "wasm no protection unsafe")
        output_run(base_speed, np_u_speed)
        csv_row.append(np_u_speed / base_speed)

        np_speed = bench(p, "_np", "wasm no protection")
        output_run(base_speed, np_speed)
        csv_row.append(np_speed / base_speed)

        bc_speed = bench(p, "_bc", "wasm bounds checked")
        output_run(base_speed, bc_speed)
        csv_row.append(bc_speed / base_speed)

        if IS_X86:
            mpx_speed = bench(p, "_mpx", "wasm mpx")
            output_run(base_speed, mpx_speed)
            csv_row.append(mpx_speed / base_speed)
        if IS_64_BIT:
            vm_speed = bench(p, "_vm", "wasm virtual memory")
            output_run(base_speed, vm_speed)
            csv_row.append(vm_speed / base_speed)
        if IS_32_BIT_X86:
            sm_speed = bench(p, "_sm", "wasm using segmentation")
            output_run(base_speed, sm_speed)
            csv_row.append(sm_speed / base_speed)
        csv_writer.writerow(csv_row)
        print("")
