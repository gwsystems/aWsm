use std::ptr;

use llvm::ffi::prelude::LLVMTypeRef;
use llvm::Compile;
use llvm::Context;
use llvm::Sub;

pub fn llvm_type_to_wasm_type(ctx: &Context, ty: &llvm::Type) -> wasmparser::Type {
    let ty_ref: LLVMTypeRef = ty.into();

    // In llvm, types have a canonical pointer, so we can compare that way
    let i32_ref: LLVMTypeRef = <i32>::get_type(ctx).into();
    if ptr::eq(ty_ref, i32_ref) {
        return wasmparser::Type::I32;
    }

    let i64_ref: LLVMTypeRef = <i64>::get_type(ctx).into();
    if ptr::eq(ty_ref, i64_ref) {
        return wasmparser::Type::I64;
    }

    let f32_ref: LLVMTypeRef = <f32>::get_type(ctx).into();
    if ptr::eq(ty_ref, f32_ref) {
        return wasmparser::Type::F32;
    }

    let f64_ref: LLVMTypeRef = <f64>::get_type(ctx).into();
    if ptr::eq(ty_ref, f64_ref) {
        return wasmparser::Type::F64;
    }

    panic!("llvm type {:?} does not have a wasm representation", ty);
}

pub fn llvm_type_to_zeroed_value<'a>(ctx: &'a Context, ty: &'a llvm::Type) -> &'a llvm::Value {
    let wasm_type = llvm_type_to_wasm_type(ctx, ty);
    wasm_type_to_zeroed_value(ctx, wasm_type)
}

pub fn wasm_type_to_llvm_type(ctx: &Context, wasm_type: wasmparser::Type) -> &llvm::Type {
    match wasm_type {
        wasmparser::Type::I32 => <i32>::get_type(ctx),
        wasmparser::Type::I64 => <i64>::get_type(ctx),
        wasmparser::Type::F32 => <f32>::get_type(ctx),
        wasmparser::Type::F64 => <f64>::get_type(ctx),
        e => panic!("wasm type {:?} does not have a llvm representation", e),
    }
}

pub fn wasm_type_to_zeroed_value(ctx: &Context, wasm_type: wasmparser::Type) -> &llvm::Value {
    match wasm_type {
        wasmparser::Type::I32 => 0i32.compile(ctx),
        wasmparser::Type::I64 => 0i64.compile(ctx),
        wasmparser::Type::F32 => 0f32.compile(ctx),
        wasmparser::Type::F64 => 0f64.compile(ctx),
        e => panic!("wasm type {:?} does not have a llvm representation", e),
    }
}

pub fn wasm_func_type_to_llvm_type<'a>(
    ctx: &'a Context,
    f_type: &wasmparser::FuncType,
) -> &'a llvm::Type {
    let return_count = f_type.returns.len();
    if return_count > 1 {
        panic!("aWsm does not support multiple return values");
    }
    let return_type = if return_count == 0 {
        <()>::get_type(ctx)
    } else {
        wasm_type_to_llvm_type(ctx, f_type.returns[0])
    };
    let mut params: Vec<&llvm::Type> = Vec::new();
    for t in &*f_type.params {
        params.push(wasm_type_to_llvm_type(ctx, *t))
    }

    llvm::FunctionType::new(return_type, &params).to_super()
}
