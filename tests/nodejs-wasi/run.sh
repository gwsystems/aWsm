#!/bin/bash

# Tests WASI syscalls via C test programs and wasi-libc
# Modified from the Node.js suite of WASI tests

make all
pushd ./vm || exit 1

declare -a tests_failed=()

./cant_dotdot_vm || tests_failed+=("cant_dotdot")
./clock_getres_vm || tests_failed+=("clock_getres")
./create_symlink.sh || tests_failed+=("create_symlink")
./exitcode.sh || tests_failed+=("exitcode")
./follow_symlink.sh || tests_failed+=("follow_symlink")
./freopen.sh || tests_failed+=("freopen")
./ftruncate.sh || tests_failed+=("ftruncate")
./getentropy.sh || tests_failed+=("getentropy")
./getrusage_vm || tests_failed+=("getrusage")
./gettimeofday_vm || tests_failed+=("gettimeofday")
./link.sh || tests_failed+=("link")
./main_args.sh || tests_failed+=("main_args")
./notdir.sh || tests_failed+=("notdir")
./poll.sh || tests_failed+=("poll")
./preopen_populates_vm || tests_failed+=("preopen_populates")
./read_file.sh || tests_failed+=("read_file")
./read_file_twice.sh || tests_failed+=("read_file_twice")
./readdir.sh || tests_failed+=("readdir")
./stat.sh || tests_failed+=("stat")
./stdin.sh || tests_failed+=("stdin")
./symlink_escape.sh || tests_failed+=("symlink_escape")
./symlink_loop.sh || tests_failed+=("symlink_loop")
./write_file.sh || tests_failed+=("write_file")

rm -rf ./data/*
popd || exit 1

for test_failed in "${tests_failed[@]}"; do
	echo "Failed $test_failed"
done

if ((${#test_failed} == 0)); then
	echo "Passed all tests!"
	exit 0
else
	exit 1
fi
