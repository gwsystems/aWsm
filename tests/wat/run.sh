#!/bin/bash

# Tests WASI syscalls via C test programs and wasi-libc

make -k clean all

declare -a tests_failed=()

./block__.minimal.out || tests_failed+=("block::*")
./block__break-inner.minimal.out || tests_failed+=("block::break-inner")
./br__.minimal.out || tests_failed+=("br::*")
./br__as-select-all.minimal.out || tests_failed+=("br::as-select-all")
./br__as-select-cond.minimal.out || tests_failed+=("br::as-select-cond")
./br__as-select-first.minimal.out || tests_failed+=("br::as-select-first")
./br__as-select-second.minimal.out || tests_failed+=("br::as-select-second")
./br__type-f32-f32.minimal.out || tests_failed+=("br::type-f32-f32")
./br__type-f32.minimal.out || tests_failed+=("br::type-f32")
./br__type-f64-f64.minimal.out || tests_failed+=("br::type-f64-f64")
./br__type-f64.minimal.out || tests_failed+=("br::type-f64")
./br__type-i32-i32.minimal.out || tests_failed+=("br::type-i32-i32")
./br__type-i32.minimal.out || tests_failed+=("br::type-i32")
./br__type-i64-i64.minimal.out || tests_failed+=("br::type-i64-i64")
./br__type-i64.minimal.out || tests_failed+=("br::type-i64")
./br_if__.minimal.out || tests_failed+=("br_if::*")
./br_table__.minimal.out || tests_failed+=("br_table::*")
./br_table__as-block-first.minimal.out || tests_failed+=("br_table::as-block-first")
./br_table__as-block-mid.minimal.out || tests_failed+=("br_table::as-block-mid")
./br_table__as-call-first.minimal.out || tests_failed+=("br_table::as-call-first")
./br_table__as-call_indirect-first.minimal.out || tests_failed+=("br_table::as-call_indirect-first")
./br_table__as-call_indirect-func.minimal.out || tests_failed+=("br_table::as-call_indirect-func")
./br_table__as-call_indirect-last.minimal.out || tests_failed+=("br_table::as-call_indirect-func")
./br_table__as-call_indirect-mid.minimal.out || tests_failed+=("br_table::as-call_indirect-func")
./br_table__as-call-last.minimal.out || tests_failed+=("br_table::as-call-last")
./br_table__as-call-mid.minimal.out || tests_failed+=("br_table::as-call-mid")
./br_table__as-compare-left.minimal.out || tests_failed+=("br_table::as-compare-left")
./br_table__as-compare-right.minimal.out || tests_failed+=("br_table::as-compare-right")
./br_table__as-convert-operand.minimal.out || tests_failed+=("br_table::as-convert-operand")
./br_table__as-global.set-value.minimal.out || tests_failed+=("br_table::as-global.set-value")
./br_table__as-if-cond.minimal.out || tests_failed+=("br_table::as-if-cond")
./br_table__as-load-address.minimal.out || tests_failed+=("br_table::as-load-address")
./br_table__as-loadN-address.minimal.out || tests_failed+=("br_table::as-loadN-address")
./br_table__as-loop-first.minimal.out || tests_failed+=("br_table::as-loop-first")
./br_table__as-loop-last.minimal.out || tests_failed+=("br_table::as-loop-last")
./br_table__as-loop-mid.minimal.out || tests_failed+=("br_table::as-loop-mid")
./br_table__as-memory.grow-size.minimal.out || tests_failed+=("br_table::as-memory.grow-size")
./br_table__as-select-first.minimal.out || tests_failed+=("br_table::as-select-first")
./br_table__as-select-second.minimal.out || tests_failed+=("br_table::as-select-second")
./br_table__as-store-address.minimal.out || tests_failed+=("br_table::as-store-address")
./br_table__as-store-value.minimal.out || tests_failed+=("br_table::as-store-value")
./br_table__as-storeN-address.minimal.out || tests_failed+=("br_table::as-storeN-address")
./br_table__as-storeN-value.minimal.out || tests_failed+=("br_table::as-storeN-value")
./br_table__type-f32-value.minimal.out || tests_failed+=("br_table::type-f32-value")
./br_table__type-f32.minimal.out || tests_failed+=("br_table::type-f32")
./br_table__type-f64.minimal.out || tests_failed+=("br_table::type-f64")
./br_table__type-i32-value.minimal.out || tests_failed+=("br_table::type-i32-value")
./br_table__type-i32.minimal.out || tests_failed+=("br_table::type-i32")
./br_table__type-i64-value.minimal.out || tests_failed+=("br_table::type-i64-value")
./br_table__type-i64.minimal.out || tests_failed+=("br_table::type-i64")
./loop__.minimal.out || tests_failed+=("loop::*")

for test_failed in "${tests_failed[@]}"; do
	echo "Failed $test_failed"
done

if ((${#test_failed} == 0)); then
	echo "Passed all tests!"
	exit 0
else
	exit 1
fi
