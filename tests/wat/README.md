# WebAssembly Specification Test Suite

This directory contains tests based on the official WebAssembly testsuite at https://github.com/WebAssembly/testsuite/. These and all original tests are licensed under the Apache-2.0 License. See the license file in this directory. There are also a handful of original custom tests written in WAT. These files are also licensed under the Apache-2.0 License.

## Test Porting Approach

WebAssembly tests are typically written in the WAST format, which is WAT extended with a handful of test assertions. Given that some of these assertions are compile-time and some are runtime, the method of test execution is more complicated for our AOT-based toolchain. Currently, compile-time assertions such as `assert_invalid` and `assert_trap` are ignored, and support is limited to `assert_return`.

The method of supporting `assert_return` is provided by manually refactoring the assertion as an imperative branch that calls the WASI exit function within a `_start` function. The exit code can be incremented to uniquely identify the failing test.

This assertion

```wat
(assert_return (invoke "as-block-first" (i32.const 0)) (i32.const 2))
```

Becomes

```
	(func (export "_start") (param i32)
        ...
        (if (i32.ne (call $as-block-first (i32.const 0)) (i32.const 2)) (then
            (call $proc_exit (i32.const 5))
        ))
        ...
    )
```

In order to invoke the tested function, the exported function needs to be refactored to have an internally callable label:

```
    (func (export "as-block-first") (param i32) (result i32)
        (block (br_if 0 (local.get 0)) (return (i32.const 2))) (i32.const 3)
    )
```

Becomes

```
	(func $as-block-first (export "as-block-first") (param i32) (result i32)
		(block (br_if 0 (local.get 0)) (return (i32.const 2))) (i32.const 3)
	)
```

Some of the unit tests require optional/proposed extensions to the core WebAssembly specification, such as multiple return values, loop parameters, or reference types. Generally, these tests and assertions are commented out with a comment identifying the required extension.'

Generally, when a test in a large file files, it is advantageous to break the failing tests out into distinct \_.wat files. This makes it simpler to examine the LLVM bitcode that aWsm generates. The name of the section of the test suite acts as a prefix followed by a double underscore `__` followed by the function getting tested. The large file containing all of the remaining functions in a section should be named `section\_\__*.wat`.

All files need to be added to the Makefile and ./run.sh driver script following the existing patterns.

## Sections Ported

| Section               | assert_return | assert_invalid | assert_trap |
| --------------------- | ------------- | -------------- | ----------- |
| address               | N             | N              | N           |
| align                 | N             | N              | N           |
| binary-leb128         | N             | N              | N           |
| binary                | N             | N              | N           |
| block                 | Y             | N              | N           |
| br                    | Y             | N              | N           |
| br_if                 | Y             | N              | N           |
| br_table              | Y             | N              | N           |
| bulk                  | N             | N              | N           |
| call                  | Y             | N              | N           |
| call_indirect         | Y             | N              | N           |
| comments              | N             | N              | N           |
| const                 | Y             | N              | N           |
| conversions           | Y             | N              | N           |
| custom                | N             | N              | N           |
| data                  | N             | N              | N           |
| elem                  | N             | N              | N           |
| endianness            | N             | N              | N           |
| exports               | N             | N              | N           |
| extract-parts         | N             | N              | N           |
| f32                   | N             | N              | N           |
| f32_bitwise           | N             | N              | N           |
| f32_cmp               | N             | N              | N           |
| f64                   | N             | N              | N           |
| f64_bitwise           | N             | N              | N           |
| f64_cmp               | N             | N              | N           |
| fac                   | N             | N              | N           |
| float_exprs           | N             | N              | N           |
| float_literals        | N             | N              | N           |
| float_memory          | N             | N              | N           |
| float_misc            | N             | N              | N           |
| forward               | N             | N              | N           |
| func                  | N             | N              | N           |
| func_ptrs             | N             | N              | N           |
| globals               | N             | N              | N           |
| i32                   | Y             | N              | N           |
| i64                   | Y             | N              | N           |
| if                    | Y             | N              | N           |
| imports               | N             | N              | N           |
| inline_module         | N             | N              | N           |
| int_exprs             | N             | N              | N           |
| int_literals          | N             | N              | N           |
| labels                | N             | N              | N           |
| left-to-right         | N             | N              | N           |
| linking               | N             | N              | N           |
| load                  | N             | N              | N           |
| local.get             | N             | N              | N           |
| local.set             | N             | N              | N           |
| local.tee             | N             | N              | N           |
| loop                  | Y             | N              | N           |
| memory                | N             | N              | N           |
| memory_copy           | N             | N              | N           |
| memory_fill           | N             | N              | N           |
| memory_grow           | N             | N              | N           |
| memory_init           | N             | N              | N           |
| memory_redundancy     | N             | N              | N           |
| memory_size           | N             | N              | N           |
| memory_trap           | N             | N              | N           |
| names                 | N             | N              | N           |
| nop                   | N             | N              | N           |
| ref_func              | N             | N              | N           |
| ref_is_null           | N             | N              | N           |
| ref_null              | N             | N              | N           |
| return                | Y             | N              | N           |
| select                | Y             | N              | N           |
| skip-stack-guard-page | N             | N              | N           |
| stack                 | N             | N              | N           |
| start                 | N             | N              | N           |
| store                 | N             | N              | N           |
| switch                | Y             | N              | N           |
| table-sub             | N             | N              | N           |
| table                 | N             | N              | N           |
| table_copy            | N             | N              | N           |
| table_fill            | N             | N              | N           |
| table_get             | N             | N              | N           |
| table_grow            | N             | N              | N           |
| table_init            | N             | N              | N           |
| table_set             | N             | N              | N           |
| table_size            | N             | N              | N           |
| token                 | N             | N              | N           |
| traps                 | N             | N              | N           |
| type                  | N             | N              | N           |
| unreachable           | N             | N              | N           |
| unreached-invalid     | N             | N              | N           |
| unreached-valid       | N             | N              | N           |
| unwind                | Y             | N              | N           |
| utf8-import-field     | N             | N              | N           |
| utf8-import-module    | N             | N              | N           |
| utf8-invalid-encoding | N             | N              | N           |
