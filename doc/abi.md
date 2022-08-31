---
geometry: margin=2cm
---

<!-- Front-matter provided to cleanup tables when generating PDF via pandoc -->

# aWsm Binary Interfaces

The goal of the aWsm/SLEdge toolchain is to perform ahead-of-time (AOT) compilation on a WebAssembly module, yielding executable native code that maintains the sandboxed execution environment of the WebAssembly specification.

# aWsm-generated LLVM bitcode \*.bc

The aWsm compiler is responsible for the first portion of this toolchain. It compiles valid WebAssembly binary into LLVM bitcode implementing the WebAssembly specification. During compilation, the aWsm compiler performs code generation for a subset of the WebAssembly specification either by calling the LLVM C++ API to generate code or by mapping to LLVM intrinsic functions that implement complex behavior. This means that the resulting \*.bc file includes code implementing certain aspects of the WebAssembly sandbox, but **not** the full specification. Substantial aspects of the WebAssembly specification are generated as unresolved external symbols that are expected to be resolved in a subsequent linking step with a runtime library.

The subset of the WebAssembly specification generated directly by the aWsm compiler includes the code related to control flow, the operand stack, the function index space, the type index space, locals, globals, and the WebAssembly instructions that map to equivalent LLVM intrinsic functions provided by the LLVM toolchain itself.

In certain cases, there is a slight behavior mismatch between a specified WebAssembly function and the code that aWsm is able to generate, typically around unusual edge cases. Because such generated code tend to be more amenable to optimization, the aWsm compiler exposes a special flag for generating "fast unsafe" code. When enabled, strict WebAssembly specification compliance is sacrificed, and a WebAssembly is mapped to an LLVM intrinsic that is equivalent in typical use cases. When disabled, the WebAssembly instruction is instead exposed as a external symbol and linked to runtime code that more strictly follows the WebAssembly specification.

## WebAssembly Instruction Implementation

Here is a list of all WebAssembly instructions as of December 2021. Each table corresponds to a section of instructions in the WebAssembly specification. Each cell captures information about how a WebAssembly instruction is implemented:

- codegen - The aWsm compiler generates backing code using the LLVM APIs and requires no intrinsic or runtime support.
- intrinsic - The aWsm compiler generates backing code using the LLVM APIs and calls an LLVM intrinsic
- extern - The aWsm compiler generates backing code using the LLVM APIs and calls an external runtime symbol
- **UNSUPPORTED** - The aWsm compiler does not support this symbol. The WebAssembly specification has grown since our implementation, and we have yet to support these new instructions. Nonetheless, based on our experience compiling various sample apps, this subset seems to handle clang and wasi-libc well.

The two columns represent the state of the resulting LLVM bitcode when the "fast unsafe" option is enabled or disabled.

### [Control Instructions](https://webassembly.github.io/spec/core/syntax/instructions.html#control-instructions)

| instruction    | "fast unsafe" enabled                 | "fast unsafe" disabled                |
| -------------- | ------------------------------------- | ------------------------------------- |
| block / end    | codegen                               | codegen                               |
| br             | codegen                               | codegen                               |
| br_if          | codegen                               | codegen                               |
| br_table       | codegen                               | codegen                               |
| call           | codegen                               | codegen                               |
| call_indirect  | extern (`get_function_from_table`)    | extern (`get_function_from_table`)    |
| if / else /end | codegen                               | codegen                               |
| loop / end     | codegen                               | codegen                               |
| nop            | codegen                               | codegen                               |
| return         | codegen                               | codegen                               |
| unreachable    | extern (`awsm_abi__trap_unreachable`) | extern (`awsm_abi__trap_unreachable`) |

### [Reference instructions](https://webassembly.github.io/spec/core/syntax/instructions.html#reference-instructions)

| instruction | "fast unsafe" enabled | "fast unsafe" disabled |
| ----------- | --------------------- | ---------------------- |
| ref.null    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| ref.is_null | **UNSUPPORTED**       | **UNSUPPORTED**        |
| ref.func    | **UNSUPPORTED**       | **UNSUPPORTED**        |

### [Parametric Instructions](https://webassembly.github.io/spec/core/syntax/instructions.html#parametric-instructions)

| instruction | "fast unsafe" enabled | "fast unsafe" disabled |
| ----------- | --------------------- | ---------------------- |
| drop        | codegen               | codegen                |
| select      | codegen               | codegen                |

### [Variable Instructions](https://webassembly.github.io/spec/core/syntax/instructions.html#variable-instructions)

| instruction | "fast unsafe" enabled                       | "fast unsafe" disabled                      |
| ----------- | ------------------------------------------- | ------------------------------------------- |
| local.get   | codegen                                     | codegen                                     |
| local.set   | codegen                                     | codegen                                     |
| local.tee   | codegen                                     | codegen                                     |
| global.get  | extern (`get_global_i32`, `get_global_i64`) | extern (`get_global_i32`, `get_global_i64`) |
| global.set  | extern (`set_global_i32`, `set_global_i64`) | extern (`set_global_i32`, `set_global_i64`) |

### [Numeric Instructions](https://webassembly.github.io/spec/core/syntax/instructions.html#numeric-instructions)

| instruction         | "fast unsafe" enabled                    | "fast unsafe" disabled                   |
| ------------------- | ---------------------------------------- | ---------------------------------------- |
| i32.add             | codegen                                  | codegen                                  |
| i32.and             | codegen                                  | codegen                                  |
| i32.clz             | intrinsic ([llvm.ctlz.i32][llvm-ctlz])   | intrinsic ([llvm.ctlz.i32][llvm-ctlz])   |
| i32.const           | codegen                                  | codegen                                  |
| i32.ctz             | intrinsic ([llvm.cttz.i32][llvm-ctlz])   | intrinsic ([llvm.cttz.i32][llvm-ctlz])   |
| i32.div_s           | codegen                                  | extern (`i32_div`)                       |
| i32.div_u           | codegen                                  | extern (`u32_div`)                       |
| i32.eq              | codegen                                  | codegen                                  |
| i32.eqz             | codegen                                  | codegen                                  |
| i32.extend8_s       | **UNSUPPORTED**                          | **UNSUPPORTED**                          |
| i32.extend16_s      | **UNSUPPORTED**                          | **UNSUPPORTED**                          |
| i32.ge_s            | codegen                                  | codegen                                  |
| i32.gt_s            | codegen                                  | codegen                                  |
| i32.le_s            | codegen                                  | codegen                                  |
| i32.lt_s            | codegen                                  | codegen                                  |
| i32.mul             | codegen                                  | codegen                                  |
| i32.ne              | codegen                                  | codegen                                  |
| i32.or              | codegen                                  | codegen                                  |
| i32.popcnt          | intrinsic ([llvm.ctpop.i32][llvm-ctpop]) | intrinsic ([llvm.ctpop.i32][llvm-ctpop]) |
| i32.reinterpret_f32 | codegen                                  | codegen                                  |
| i32.rem_s           | codegen                                  | extern (`i32_rem`)                       |
| i32.rem_u           | codegen                                  | extern (`u32_rem`)                       |
| i32.rotl            | extern (`rotl_u32`)                      | extern (`rotl_u32`)                      |
| i32.rotr            | extern (`rotr_u32`)                      | extern (`rotr_u32`)                      |
| i32.shl             | codegen                                  | codegen                                  |
| i32.shr_s           | codegen                                  | codegen                                  |
| i32.shr_u           | codegen                                  | codegen                                  |
| i32.sub             | codegen                                  | codegen                                  |
| i32.trunc_f32_s     | codegen                                  | extern (`i32_trunc_f32`)                 |
| i32.trunc_f32_u     | codegen                                  | extern (`u32_trunc_f32`)                 |
| i32.trunc_f64_s     | codegen                                  | extern (`i32_trunc_f64`)                 |
| i32.trunc_f64_u     | codegen                                  | extern (`u32_trunc_f64`)                 |
| i32.trunc_sat_f32_s | **UNSUPPORTED**                          | **UNSUPPORTED**                          |
| i32.trunc_sat_f32_u | **UNSUPPORTED**                          | **UNSUPPORTED**                          |
| i32.trunc_sat_f64_s | **UNSUPPORTED**                          | **UNSUPPORTED**                          |
| i32.trunc_sat_f64_u | **UNSUPPORTED**                          | **UNSUPPORTED**                          |
| i32.wrap_i64        | codegen                                  | codegen                                  |
| i32.xor             | codegen                                  | codegen                                  |
| i64.add             | codegen                                  | codegen                                  |
| i64.and             | codegen                                  | codegen                                  |
| i64.clz             | intrinsic ([llvm.ctlz.i64][llvm-ctlz])   | intrinsic ([llvm.ctlz.i64][llvm-ctlz])   |
| i64.const           | codegen                                  | codegen                                  |
| i64.ctz             | intrinsic ([llvm.cttz.i64][llvm-ctlz])   | intrinsic ([llvm.cttz.i64][llvm-ctlz])   |
| i64.div_s           | codegen                                  | extern (`i64_div`)                       |
| i64.div_u           | codegen                                  | extern (`u64_div`)                       |
| i64.eq              | codegen                                  | codegen                                  |
| i64.eqz             | codegen                                  | codegen                                  |
| i64.extend_i32_s    | codegen                                  | codegen                                  |
| i64.extend_i32_u    | codegen                                  | codegen                                  |
| i64.extend8_s       | **UNSUPPORTED**                          | **UNSUPPORTED**                          |
| i64.extend16_s      | **UNSUPPORTED**                          | **UNSUPPORTED**                          |
| i64.extend32_s      | **UNSUPPORTED**                          | **UNSUPPORTED**                          |
| i64.ge_u            | codegen                                  | codegen                                  |
| i64.gt_u            | codegen                                  | codegen                                  |
| i64.le_u            | codegen                                  | codegen                                  |
| i64.lt_u            | codegen                                  | codegen                                  |
| i64.mul             | codegen                                  | codegen                                  |
| i64.ne              | codegen                                  | codegen                                  |
| i64.or              | codegen                                  | codegen                                  |
| i64.popcnt          | intrinsic ([llvm.ctpop.i64][llvm-ctpop]) | intrinsic ([llvm.ctpop.i64][llvm-ctpop]) |
| i64.reinterpret_f64 | codegen                                  | codegen                                  |
| i64.rem_s           | codegen                                  | extern (`i64_rem`)                       |
| i64.rem_u           | codegen                                  | extern (`u64_rem`)                       |
| i64.rotl            | extern (`rotl_u64`)                      | extern (`rotl_u64`)                      |
| i64.rotr            | extern (`rotr_u64`)                      | extern (`rotr_u64`)                      |
| i64.shl             | codegen                                  | codegen                                  |
| i64.shr_u           | codegen                                  | codegen                                  |
| i64.shr_s           | codegen                                  | codegen                                  |
| i64.sub             | codegen                                  | codegen                                  |
| i64.trunc_f32_s     | extern (`i64_trunc_f32`)                 | extern (`i64_trunc_f32`)                 |
| i64.trunc_f32_u     | extern (`u64_trunc_f32`)                 | extern (`u64_trunc_f32`)                 |
| i64.trunc_f64_s     | extern (`i64_trunc_f64`)                 | extern (`i64_trunc_f64`)                 |
| i64.trunc_f64_u     | extern (`u64_trunc_f64`)                 | extern (`u64_trunc_f64`)                 |
| i64.trunc_sat_f32_s | **UNSUPPORTED**                          | **UNSUPPORTED**                          |
| i64.trunc_sat_f32_u | **UNSUPPORTED**                          | **UNSUPPORTED**                          |
| i64.trunc_sat_f64_s | **UNSUPPORTED**                          | **UNSUPPORTED**                          |
| i64.trunc_sat_f64_u | **UNSUPPORTED**                          | **UNSUPPORTED**                          |
| i64.xor             | codegen                                  | codegen                                  |
| f32.abs             | intrinsic ([llvm.fabs.f32][llvm-fabs])   | intrinsic ([llvm.fabs.f32][llvm-fabs])   |
| f32.add             | codegen                                  | codegen                                  |
| f32.ceil            | extern (`f32_ceil`)                      | extern (`f32_ceil`)                      |
| f32.const           | codegen                                  | codegen                                  |
| f32.convert.i32_s   | codegen                                  | codegen                                  |
| f32.convert.i32_u   | codegen                                  | codegen                                  |
| f32.convert.i64_s   | codegen                                  | codegen                                  |
| f32.convert.i64_u   | codegen                                  | codegen                                  |
| f32.copysign        | extern (`f32_copysign`)                  | extern (`f32_copysign`)                  |
| f32.demote_f64      | codegen                                  | codegen                                  |
| f32.div             | codegen                                  | codegen                                  |
| f32.eq              | codegen                                  | codegen                                  |
| f32.floor           | extern (`f32_floor`)                     | extern (`f32_floor`)                     |
| f32.ge              | codegen                                  | codegen                                  |
| f32.gt              | codegen                                  | codegen                                  |
| f32.le              | codegen                                  | codegen                                  |
| f32.lt              | codegen                                  | codegen                                  |
| f32.max             | extern (`f32_max`)                       | extern (`f32_max`)                       |
| f32.min             | extern (`f32_min`)                       | extern (`f32_min`)                       |
| f32.mul             | codegen                                  | codegen                                  |
| f32.ne              | codegen                                  | codegen                                  |
| f32.nearest         | extern (`f32_nearest`)                   | extern (`f32_nearest`)                   |
| f32.neg             | codegen                                  | codegen                                  |
| f32.reinterpret_i32 | codegen                                  | codegen                                  |
| f32.sqrt            | intrinsic ([llvm.sqrt.f32][llvm-sqrt])   | intrinsic ([llvm.sqrt.f32][llvm-sqrt])   |
| f32.sub             | codegen                                  | codegen                                  |
| f32.trunc           | extern (`f32_trunc_f32`)                 | extern (`f32_trunc_f32`)                 |
| f64.abs             | intrinsic ([llvm.fabs.f64][llvm-fabs])   | intrinsic ([llvm.fabs.f64][llvm-fabs])   |
| f64.add             | codegen                                  | codegen                                  |
| f64.ceil            | extern (`f64_ceil`)                      | extern (`f64_ceil`)                      |
| f64.const           | codegen                                  | codegen                                  |
| f64.convert.i32_s   | codegen                                  | codegen                                  |
| f64.convert.i32_u   | codegen                                  | codegen                                  |
| f64.convert.i64_s   | codegen                                  | codegen                                  |
| f64.convert.i64_u   | codegen                                  | codegen                                  |
| f64.div             | codegen                                  | codegen                                  |
| f64.eq              | codegen                                  | codegen                                  |
| f64.floor           | extern (`f64_floor`)                     | extern (`f64_floor`)                     |
| f64.ge              | codegen                                  | codegen                                  |
| f64.gt              | codegen                                  | codegen                                  |
| f64.le              | codegen                                  | codegen                                  |
| f64.lt              | codegen                                  | codegen                                  |
| f64.max             | extern (`f64_max`)                       | extern (`f64_max`)                       |
| f64.min             | extern (`f64_min`)                       | extern (`f64_min`)                       |
| f64.mul             | codegen                                  | codegen                                  |
| f64.ne              | codegen                                  | codegen                                  |
| f64.nearest         | extern (`f64_nearest`)                   | extern (`f64_nearest`)                   |
| f64.neg             | codegen                                  | codegen                                  |
| f64.promote_f32     | codegen                                  | codegen                                  |
| f64.reinterpret_i64 | codegen                                  | codegen                                  |
| f64.sqrt            | intrinsic ([llvm.sqrt.f64][llvm-sqrt])   | intrinsic ([llvm.sqrt.f64][llvm-sqrt])   |
| f64.sub             | codegen                                  | codegen                                  |
| f64.trunc           | extern (`f64_trunc_f64`)                 | extern (`f64_trunc_f64`)                 |

### [Memory Instructions](https://webassembly.github.io/spec/core/syntax/instructions.html#memory-instructions)

| instruction       | "fast unsafe" enabled              | "fast unsafe" disabled             |
| ----------------- | ---------------------------------- | ---------------------------------- |
| data.drop         | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| i32.load          | extern (`get_i32`)                 | extern (`get_i32`)                 |
| i32.load8_s       | extern (`get_i8`)                  | extern (`get_i8`)                  |
| i32.load8_u       | extern (`get_i8`)                  | extern (`get_i8`)                  |
| i32.load16_s      | extern (`get_i16`)                 | extern (`get_i16`)                 |
| i32.load16_u      | extern (`get_i16`)                 | extern (`get_i16`)                 |
| i32.store         | extern (`set_i32`)                 | extern (`set_i32`)                 |
| i32.store8        | extern (`set_i8`)                  | extern (`set_i8`)                  |
| i32.store16       | extern (`set_i16`)                 | extern (`set_i16`)                 |
| i64.load          | extern (`get_i64`)                 | extern (`get_i64`)                 |
| i64.load8_s       | extern (`get_i8`)                  | extern (`get_i8`)                  |
| i64.load8_u       | extern (`get_i8`)                  | extern (`get_i8`)                  |
| i64.load16_s      | extern (`get_i16`)                 | extern (`get_i16`)                 |
| i64.load16_u      | extern (`get_i16`)                 | extern (`get_i16`)                 |
| i64.load32_s      | extern (`get_i32`)                 | extern (`get_i32`)                 |
| i64.load32_u      | extern (`get_i32`)                 | extern (`get_i32`)                 |
| i64.store         | extern (`set_i64`)                 | extern (`set_i64`)                 |
| i64.store8        | extern (`set_i8`)                  | extern (`set_i8`)                  |
| i64.store16       | extern (`set_i16`)                 | extern (`set_i16`)                 |
| i64.store32       | extern (`set_i32`)                 | extern (`set_i32`)                 |
| f32.load          | extern (`get_f32`)                 | extern (`get_f32`)                 |
| f32.store         | extern (`set_f32`)                 | extern (`set_f32`)                 |
| f64.load          | extern (`get_f64`)                 | extern (`get_f64`)                 |
| f64.store         | extern (`set_f64`)                 | extern (`set_f64`)                 |
| memory.copy       | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| memory.fill       | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| memory.grow       | extern (`instruction_memory_grow`) | extern (`instruction_memory_size`) |
| memory.init       | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| memory.size       | extern (`instruction_memory_size`) | extern (`instruction_memory_size`) |
| v128.load         | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load8x8_s    | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load8x8_u    | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load16x4_s   | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load16x4_u   | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load32x2_s   | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load32x2_u   | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load32_zero  | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load64_zero  | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load8_splat  | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load16_splat | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load32_splat | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load64_splat | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load8_lane   | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load16_lane  | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load32_lane  | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.load64_lane  | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.store        | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.store8_lane  | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.store16_lane | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.store32_lane | **UNSUPPORTED**                    | **UNSUPPORTED**                    |
| v128.store64_lane | **UNSUPPORTED**                    | **UNSUPPORTED**                    |

### [Table instructions](https://webassembly.github.io/spec/core/syntax/instructions.html#table-instructions)

| instruction | "fast unsafe" enabled | "fast unsafe" disabled |
| ----------- | --------------------- | ---------------------- |
| table.get   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| table.set   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| table.size  | **UNSUPPORTED**       | **UNSUPPORTED**        |
| table.grow  | **UNSUPPORTED**       | **UNSUPPORTED**        |
| table.fill  | **UNSUPPORTED**       | **UNSUPPORTED**        |
| table.copy  | **UNSUPPORTED**       | **UNSUPPORTED**        |
| table.init  | **UNSUPPORTED**       | **UNSUPPORTED**        |
| elem.drop   | **UNSUPPORTED**       | **UNSUPPORTED**        |

### [Vector instructions](https://webassembly.github.io/spec/core/syntax/instructions.html#vector-instructions)

| instruction                   | "fast unsafe" enabled | "fast unsafe" disabled |
| ----------------------------- | --------------------- | ---------------------- |
| v128.all_true                 | **UNSUPPORTED**       | **UNSUPPORTED**        |
| v128.and                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| v128.andnot                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| v128.any_true                 | **UNSUPPORTED**       | **UNSUPPORTED**        |
| v128.bitselect                | **UNSUPPORTED**       | **UNSUPPORTED**        |
| v128.const                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| v128.not                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| v128.or                       | **UNSUPPORTED**       | **UNSUPPORTED**        |
| v128.xor                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.add_sat_s               | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.add_sat_u               | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.all_true                | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.abs                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.avgr_u                  | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.bitmask                 | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.extract_lane_s          | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.extract_lane_u          | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.eq                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.le_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.le_u                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.lt_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.lt_u                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.ge_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.ge_u                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.gt_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.gt_u                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.max_s                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.max_u                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.min_s                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.min_u                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.ne                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.neg                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.popcnt                  | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.replace_lane            | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.shuffle                 | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.sub_sat_s               | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.sub_sat_u               | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.swizzle                 | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i8x16.splat                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.abs                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.add_sat_s               | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.add_sat_u               | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.all_true                | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.avgr_u                  | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.bitmask                 | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.eq                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.extadd_pairwise_i8x16_s | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.extadd_pairwise_i8x16_u | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.extmul_high_i8x16_u     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.extmul_high_i8x16_s     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.extmul_low_i8x16_u      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.extmul_low_i8x16_s      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.extract_lane_s          | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.extract_lane_u          | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.ge_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.ge_u                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.gt_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.gt_u                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.le_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.le_u                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.lt_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.lt_u                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.max_s                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.max_u                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.min_s                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.min_u                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.mul                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.ne                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.neg                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.q15mulr_sat_s           | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.replace_lane            | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.splat                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.sub_sat_s               | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i16x8.sub_sat_u               | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.abs                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.all_true                | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.bitmask                 | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.dot_i16x8_s             | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.eq                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.extadd_pairwise_i16x8_s | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.extadd_pairwise_i16x8_u | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.extmul_low_i16x8_u      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.extmul_low_i16x8_s      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.extmul_high_i16x8_u     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.extmul_high_i16x8_s     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.extract_lane            | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.ge_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.ge_u                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.gt_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.gt_u                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.le_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.le_u                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.lt_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.lt_u                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.max_u                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.max_s                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.min_u                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.min_s                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.mul                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.ne                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.neg                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.replace_lane            | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.splat                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.trunc_sat_f32x4_s       | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.trunc_sat_f32x4_u       | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.trunc_sat_f64x2_s_zero  | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i32x4.trunc_sat_f64x2_u_zero  | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.abs                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.all_true                | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.bitmask                 | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.eq                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.extract_lane            | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.extmul_high_i32x4_s     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.extmul_high_i32x4_u     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.extmul_low_i32x4_s      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.extmul_low_i32x4_u      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.mul                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.ne                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.neg                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.le_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.lt_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.ge_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.gt_s                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.replace_lane            | **UNSUPPORTED**       | **UNSUPPORTED**        |
| i64x2.splat                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.abs                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.add                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.ceil                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.convert_i32x4.s         | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.convert_i32x4.u         | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.demote_f64x2_zero       | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.div                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.eq                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.extract_lane            | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.floor                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.ge                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.gt                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.le                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.lt                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.ne                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.max                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.min                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.mul                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.nearest                 | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.neg                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.pmax                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.pmin                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.replace_lane            | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.splat                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.sqrt                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.sub                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f32x4.trunc                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.abs                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.add                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.ceil                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.convert_low_i32x4_s     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.convert_low_i32x4_u     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.div                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.eq                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.extract_lane            | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.floor                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.ge                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.gt                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.le                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.lt                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.max                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.min                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.mul                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.ne                      | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.nearest                 | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.neg                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.pmax                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.pmin                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.promote_low_f32x4       | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.replace_lane            | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.splat                   | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.sqrt                    | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.sub                     | **UNSUPPORTED**       | **UNSUPPORTED**        |
| f64x2.trunc                   | **UNSUPPORTED**       | **UNSUPPORTED**        |

[llvm-ctlz]: https://llvm.org/docs/LangRef.html#llvm-ctlz-intrinsic
[llvm-ctlz]: https://llvm.org/docs/LangRef.html#llvm-ctlz-intrinsic
[llvm-ctpop]: https://llvm.org/docs/LangRef.html#llvm-ctpop-intrinsic
[llvm-fabs]: https://llvm.org/docs/LangRef.html#llvm-fabs-intrinsic
[llvm-sqrt]: https://llvm.org/docs/LangRef.html#llvm-sqrt-intrinsic
[llvm-trap]: https://llvm.org/docs/LangRef.html#llvm-trap-intrinsic

# aWsm Module Instantiation

Prior to execution, a WebAssembly module undergoes a process called instantiation that generates a unique execution context. The WebAssembly specification outlines detailed steps for how such instantiation should occur, but aWsm generally presents a significantly simpler process based around native symbols.

## Imports

WebAssembly function imports are simply concatenated in the format `{module}_{name}` and written to as unresolved external symbols in the resulting \*.bc file. This does not allow indirection via function pointers and is assumed to be predominantly use to link to WASI syscalls. This simple technique means that that the module can theoretically call any ELF symbol that it has visibility to. This needs to be manually inspected to prevent security issues. If the symbol of the expected import is not properly resolved during a subsequent linking step, a linking error occurs.

## Exports

WebAssembly functions and globals are generated as symbols with the provided export name prepended with either `wasmf_{export_name}` or `wasmg_{export_name}` depending on if they are are function or export.

## Non-exported Functions and Globals

Non exported functions and globals are generated as symbols `wasmf_internal_{identifier}` or `wasmg_internal_{identifier}`. The identifier is typically an integer, but it can also be the actual name of the source function or global if the optional WebAssembly Name Section was provided. This is typically enabled by compiling the source \*.wasm module with the `-g` flag.

The globals might be inlined based on the flags passed to the aWsm compiler. See `Globals Initialization` above for further information.

## Table Initialization

An aWsm \*.bc file expects an external entity to allocate an indirect table for function pointers. The WebAssembly specification expects that initialization should occur via the use of table and reference instructions. However, aWsm does not currently support these instructions. Rather, the aWsm compiler generates a function called `populate_table` that repeatedly calls the external symbol `add_function_to_table` to populate the table. This `populate_table` function is thus functionally equivalent to the `table.init` instruction. The external `add_function_to_table` symbol is functionally equivalent to `table.set`. The aWsm generated `call_indirect` logic invokes an external symbol `get_function_from_table` that is functionally equivalent to `table.get`.

## Globals Initialization

A WebAssembly instance is specified as storing globals in a dynamic vector capable of storing WebAssembly numbers, vectors, or references. Elements within this vector are identified by an index of types `u32`. See [the spec](https://webassembly.github.io/spec/core/syntax/modules.html#globals) for further details.

The aWsm compiler currently only supports getting and setting integral `i32` and `i64` types. When a module is encountered that attempts to set a `f32`, `f64`, `v128`, or reference as a global, the compiler fails with the error message `Unimplemented runtime global type`.

The aWsm compiler generates a `populate_globals` function in resulting \*.bc files responsible for populating the initial values of the WebAssembly globals. The behavior by which `populate_globals` accomplishes this is set by compiletime aWsm flags.

The `runtime-globals` flag indicates whether globals should be populated via external symbols provided by a linked runtime.

When `runtime-globals` is not set, the `populate_globals` functions generates WebAssembly globals as LLVM globals. This results in the globals being written to the \*.bc file with at symbols with the prefix `wasmg_`. This simplifies the requirements of the embedding environment for small environments such as microcontrollers, but it assumes that a single module is not instantiated multiple times, as different instances of a module end up sharing mutable state. If multiple instances are required, it is assumed that the author copies a translation unit multiple times and rewrites ELF symbols to avoid symbol collisions. When `runtime-globals` flag is not set, the `inline-constant-globals` flag may also optionally be set, which allows immutable globals to avoid generating a `wasmg_` global symbol and instead be aggressively inlined in the resulting native code. Mutable globals variables are still generated as LLVM globals with a `wasmg_` global symbol.

When `runtime-globals` is set, a \*.bc file output by the aWsm compiler does not make assumptions about implementation details for what a WebAssembly Globals vector looks like and it calls an abstract interface of external symbols in the following format:

```c
int32_t get_global_i32(uint32_t idx);
int64_t get_global_i64(uint32_t idx);
void set_global_i32(uint32_t idx, int32_t value);
void set_global_i64(uint32_t idx, int64_t value);
```

The `populate_globals` function assumes that runtime has implemented this interface and preallocated the backing storage for the globals vector. It calls `set_global_i32` and `set_global_i64` for all globals.

## Memory Initialization

The aWsm compiler expects a WebAssembly module to declare a single WebAssembly memory. Zero or multiple memories cause the compiler to panic. The compiler transforms the initial and max WebAssembly pages values provided in this WebAssembly instruction into the LLVM constant globals `starting_pages` and `max_pages`. If a max value is not provided, the value defaults to zero, which indicates that there is no cap.

The compiler generates an initialization function for each defined segment in the format `init_memory_offset_0`, `init_memory_offset_1`, etc. They return the target offset where the associated segment should be copied into the Linear Memory, respectively.

It also generates a `populate_memory` function, which:

1. Calls the offset functions above to get the target offset in the linear memory
2. generates globals at `init_vector_0`, `init_vector_1`, etc. containing to data segment we want to write to linear memory
3. calls an undefined external symbol `initialize_region` for each offset/data pair to copy the data stored in `init_vector_0` to the target offset returned by `init_memory_offset_0`.

The net result is that `populate_memory` should copy all relevant data into the WebAssembly linear memory.

## Wasmception Entrypoint

If a WebAssembly module is compiled using wasmception (a libc that predates WASI), the runtime must also initialize libc manually by calling `wasmf___init_libc`. The entrypoint is then available at `wasmf_main`, which is directly called with arguments.

## WASI Entrypoint

If a WebAssembly module is compiled using WASI, the runtime must initialize a WASI context with arguments and other options. This is expected to be present as a global in your runtime. This API for this depends on your WASI implementation. Once this is set, a runtime calls the entrypoint `_start`, which is exposed as `wasmf__start`.

## aWsm Application Binary Interface

Based on the above information about how aWsm deals with WebAssembly instructions and initialization, it is clear that in order to be able to execute the LLVM bitcode emitted by aWsm, the \*.bc file needs to be linked with \*.c translation units or a library that implements the unresolved external symbols listed above. This set of external symbols constitutes an ABI (henceforth called the "aWsm ABI").

The following header contains all of the external symbols required of a supporting runtime library to provide the expected level of support outlined above. This ABI would expand to add support to new WebAssembly instructions.

```c
/* aWsm Table Initialization */
void add_function_to_table(uint32_t idx, uint32_t type_id, char *pointer);

/* aWsm Memory Initialization */
void initialize_region(uint32_t offset, uint32_t region_size, uint8_t region[region_size]);

/* Control Instructions */

/* call_indirect */
char *get_function_from_table(uint32_t idx, uint32_t type_id);

/* Variable Instructions */

/* global.get when runtime-globals is set */
int32_t get_global_i32(uint32_t idx);
int64_t get_global_i64(uint32_t idx);

/* global.set and populate_globals when runtime-globals is set */
void set_global_i32(uint32_t idx, int32_t value);
void set_global_i64(uint32_t idx, int64_t value);


/* Numeric Instructions (https://webassembly.github.io/spec/core/syntax/instructions.html#numeric-instructions) */

/* i32.div_s if not compiled with use_fast_unsafe option */
int32_t i32_div(int32_t a, int32_t b);
/* i32.div_u if not compiled with use_fast_unsafe option */
uint32_t u32_div(uint32_t a, uint32_t b);
/* i32.rem_s if not compiled with use_fast_unsafe option */
int32_t i32_rem(int32_t a, int32_t b);
/* i32.rem_u if not compiled with use_fast_unsafe option */
uint32_t u32_rem(uint32_t a, uint32_t b);
/* i32.rotl */
uint32_t rotl_u32(uint32_t n, uint32_t c_u32);
/* i32.rotr */
uint32_t rotr_u32(uint32_t n, uint32_t c_u32);
/* i32.trunc_f32_s if not compiled with use_fast_unsafe option */
int32_t i32_trunc_f32(float f);
/* i32.trunc_f32_u if not compiled with use_fast_unsafe option */
uint32_t u32_trunc_f32(float f);
/* i32.trunc_f64_s if not compiled with use_fast_unsafe option */
int32_t i32_trunc_f64(double f);
/* i32.trunc_f32_u if not compiled with use_fast_unsafe option */
uint32_t u32_trunc_f64(double f);
/* i64.div_s if not compiled with use_fast_unsafe option */
int64_t i64_div(int64_t a, int64_t b);
/* i64.div_u if not compiled with use_fast_unsafe option */
uint64_t u64_div(uint64_t a, uint64_t b);
/* i64.rem_s if not compiled with use_fast_unsafe option */
int64_t i64_rem(int64_t a, int64_t b);
/* i64.rem_u if not compiled with use_fast_unsafe option */
uint64_t u64_rem(uint64_t a, uint64_t b);
/* i64.rotl */
uint64_t rotl_u64(uint64_t n, uint64_t c_u64)
/* i64.rotr */
uint64_t rotr_u64(uint64_t n, uint64_t c_u64);
/* i64.trunc_f32_s */
uint64_t u64_trunc_f32(float f);
/* i64.trunc_f32_u */
int64_t i64_trunc_f32(float f);
/* i64.trunc_f64_s */
int64_t i64_trunc_f64(double f);
/* i64.trunc_f64_u */
uint64_t u64_trunc_f64(double f);
/* f32.ceil */
float f32_ceil(float a);
/* f32.copysign */
float f32_copysign(float a, float b);
/* f32.floor  */
float f32_floor(float a);
/* f32.max */
float f32_max(float a, float b);
/* f32.min */
float f32_min(float a, float b);
/* f32.nearest */
float f32_nearest(float a);
/* f32.trunc */
float f32_trunc_f32(float f);
/* f64.ceil */
double f64_ceil(double a);
/* f64.floor */
double f64_floor(double a);
/* f64.max */
double f64_max(double a, double b);
/* f64.min */
double f64_min(double a, double b);
/* f64.nearest */
double f64_nearest(double a)
/* f64.trunc */
double f64_trunc_f64(float double);

/* Memory Instructions */
/* i32.load8_s, i32.load8_u, i64.load8_s, i64.load8_u */
int8_t get_i8(uint32_t offset);
/* i32.load16_s, i32.load16_u, i64.load16_s, i64.load16_u */
int16_t get_i16(uint32_t offset);
/* i32.load, i64.load32_s, i64.load32_u  */
int32_t get_i32(uint32_t offset);
/* i32.store8, i64.store8 */
void set_i8(uint32_t offset, int8_t value);
/* i32.store16, i64.store16 */
void set_i16(uint32_t offset, int16_t value);
/* i32.store, i64.store32 */
void set_i32(uint32_t offset, int32_t value);
/* i64.load */
int64_t get_i64(uint32_t offset);
/* i64.store */
void set_i64(uint32_t offset, int64_t value);
/* f32.load */
float get_f32(uint32_t offset);
/* f32.store */
void set_f32(uint32_t offset, float value)
/* f64.load */
double get_f64(uint32_t offset);
/* f64.store */
void set_f64(uint32_t offset, double value);
/* memory.size */
uint32_t instruction_memory_size(void);
/* memory.grow */
int32_t instruction_memory_grow(uint32_t count);
```

In addition to the above, the following symbols are reserved for the aWsm compiler:

- populate_table
- populate_globals
- populate_memory
- wasmf_internal\_\*
- wasmg_internal\_\*
- wasmf\_\*
- wasmg\_\*
- init_memory_offset\_\*
- init_vector\_\*
- starting_pages
- max_pages
