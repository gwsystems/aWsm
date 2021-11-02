#!/bin/bash

# Tests WASI syscalls via C test programs and wasi-libc

declare -a tests_failed=()
declare tests=""
declare build_failed=false

# Takes an argument for the backing to run the tests against (wasmception, uvwasi, minimal)

case ${1:-all} in
	minimal)
		make -k minimal.out || build_failed=true
		tests="*.minimal.out"
		;;
	uvwasi)
		make -k uvwasi.out || build_failed=true
		tests="*.uvwasi.out"
		;;
	wasmception)
		make -k wasmception.out || build_failed=true
		tests="*.wasmception.out"
		;;
	all)
		tests="*.out"
		make -k all || build_failed=true
		;;
	*)
		echo "Usage: $0 {minimal|uvwasi|wasmception|all}"
		echo "Defaults to all if no arg provided"
		exit 1
		;;
esac

for test_failed in "${tests_failed[@]}"; do
	echo "Failed $test_failed"
done

if $build_failed; then
	echo "Failed to build all tests!"
fi

if  ((${#test_failed} > 0)); then
	echo "Failed to execute all tests:"
	for i in $(find . -name "${tests}"); do
		"$i" || {
			i="${i/%.out/}"
			i="${i/%.wasmception/ (wasmception)}"
			i="${i/%.uvwasi/ (uvwasi)}"
			i="${i/%.minimal/ (minimal)}"
			i="${i/#.\//}"
			i="${i/__/::}"
			tests_failed+=("$i")
		}
	done
fi

if ! $build_failed && ((${#test_failed} == 0)); then
	echo "Built and passed all tests!"
	exit 0
else
	exit 1
fi
