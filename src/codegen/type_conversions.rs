use std::ptr;

use llvm::ffi::prelude::LLVMTypeRef;
use llvm::Context;

use crate::codegen::llvm_wrapper::{BasicType, BasicValue, FunctionType};

pub fn llvm_type_to_wasm_type(ctx: &Context, ty: &llvm::Type) -> wasmparser::Type {
    // FIXME: Move this to the `llvm_wrapper`
    let ty_ref: LLVMTypeRef = ty.into();

    // In llvm, types have a canonical pointer, so we can compare that way
    let i32_ref: LLVMTypeRef = BasicType::i32_ty(ctx).itype().into();
    if ptr::eq(ty_ref, i32_ref) {
        return wasmparser::Type::I32;
    }

    let i64_ref: LLVMTypeRef = BasicType::i64_ty(ctx).itype().into();
    if ptr::eq(ty_ref, i64_ref) {
        return wasmparser::Type::I64;
    }

    let f32_ref: LLVMTypeRef = BasicType::f32_ty(ctx).itype().into();
    if ptr::eq(ty_ref, f32_ref) {
        return wasmparser::Type::F32;
    }

    let f64_ref: LLVMTypeRef = BasicType::f64_ty(ctx).itype().into();
    if ptr::eq(ty_ref, f64_ref) {
        return wasmparser::Type::F64;
    }

    panic!("llvm type {:?} does not have a wasm representation", ty);
}

pub fn llvm_type_to_zeroed_value<'a>(ctx: &'a Context, ty: &'a llvm::Type) -> BasicValue<'a> {
    let wasm_type = llvm_type_to_wasm_type(ctx, ty);
    wasm_type_to_zeroed_value(ctx, wasm_type)
}

pub fn wasm_type_to_llvm_type(ctx: &Context, wasm_type: wasmparser::Type) -> BasicType {
    match wasm_type {
        wasmparser::Type::I32 => BasicType::i32_ty(ctx),
        wasmparser::Type::I64 => BasicType::i64_ty(ctx),
        wasmparser::Type::F32 => BasicType::f32_ty(ctx),
        wasmparser::Type::F64 => BasicType::f64_ty(ctx),
        e => panic!("wasm type {:?} does not have a llvm representation", e),
    }
}

pub fn wasm_type_to_zeroed_value(ctx: &Context, wasm_type: wasmparser::Type) -> BasicValue {
    match wasm_type {
        wasmparser::Type::I32 => BasicValue::i32_zero(ctx),
        wasmparser::Type::I64 => BasicValue::i64_zero(ctx),
        wasmparser::Type::F32 => BasicValue::f32_zero(ctx),
        wasmparser::Type::F64 => BasicValue::f64_zero(ctx),
        e => panic!("wasm type {:?} does not have a llvm representation", e),
    }
}

pub fn wasm_func_type_to_llvm_type<'a>(
    ctx: &'a Context,
    f_type: &wasmparser::FuncType,
) -> FunctionType<'a> {
    let return_count = f_type.returns.len();
    assert!(return_count <= 1);
    let return_type = if return_count == 0 {
        BasicType::void_ty(ctx)
    } else {
        wasm_type_to_llvm_type(ctx, f_type.returns[0])
    };

    let mut params = Vec::new();
    for t in &*f_type.params {
        params.push(wasm_type_to_llvm_type(ctx, *t))
    }

    FunctionType::new(ctx, &params, return_type)
}
