use llvm::Compile;
use llvm::Context as LLVMCtx;
use llvm::Function;
use llvm::FunctionType;
use llvm::Module as LLVMModule;
use llvm::PointerType;
use llvm::Sub;

use crate::codegen::ModuleCtx;
use crate::codegen::Opt;

// Backing functions for wasm operations
pub const INITIALIZE_REGION_STUB: &str = "initialize_region";

pub const MEMORY_SIZE: &str = "instruction_memory_size";
pub const MEMORY_GROW: &str = "instruction_memory_grow";

pub const GET_F32: &str = "get_f32";
pub const SET_F32: &str = "set_f32";

pub const GET_F64: &str = "get_f64";
pub const SET_F64: &str = "set_f64";

pub const GET_I8: &str = "get_i8";
pub const SET_I8: &str = "set_i8";

pub const GET_I16: &str = "get_i16";
pub const SET_I16: &str = "set_i16";

pub const GET_I32: &str = "get_i32";
pub const SET_I32: &str = "set_i32";

pub const GET_I64: &str = "get_i64";
pub const SET_I64: &str = "set_i64";

pub const TABLE_ADD: &str = "add_function_to_table";
pub const TABLE_FETCH: &str = "get_function_from_table";

pub const GET_GLOBAL_I32: &str = "get_global_i32";
pub const SET_GLOBAL_I32: &str = "set_global_i32";

pub const GET_GLOBAL_I64: &str = "get_global_i64";
pub const SET_GLOBAL_I64: &str = "set_global_i64";

pub const I32_ROTL: &str = "rotl_u32";
pub const I32_ROTR: &str = "rotr_u32";

pub const I64_ROTL: &str = "rotl_u64";
pub const I64_ROTR: &str = "rotr_u64";

pub const I32_TRUNC_F32: &str = "i32_trunc_f32";
pub const U32_TRUNC_F32: &str = "u32_trunc_f32";
pub const I32_TRUNC_F64: &str = "i32_trunc_f64";
pub const U32_TRUNC_F64: &str = "u32_trunc_f64";

pub const I64_TRUNC_F32: &str = "i64_trunc_f32";
pub const U64_TRUNC_F32: &str = "u64_trunc_f32";
pub const I64_TRUNC_F64: &str = "i64_trunc_f64";
pub const U64_TRUNC_F64: &str = "u64_trunc_f64";

pub const F32_TRUNC_F32: &str = "f32_trunc_f32";
pub const F64_TRUNC_F64: &str = "f64_trunc_f64";

pub const U32_DIV: &str = "u32_div";
pub const U32_REM: &str = "u32_rem";

pub const I32_DIV: &str = "i32_div";
pub const I32_REM: &str = "i32_rem";

pub const U64_DIV: &str = "u64_div";
pub const U64_REM: &str = "u64_rem";

pub const I64_DIV: &str = "i64_div";
pub const I64_REM: &str = "i64_rem";

pub const F32_NEAREST: &str = "f32_nearest";
pub const F32_COPYSIGN: &str = "f32_copysign";

pub const F64_NEAREST: &str = "f64_nearest";
pub const F64_COPYSIGN: &str = "f64_copysign";

pub const TRAP_UNREACHABLE: &str = "awsm_abi__trap_unreachable";

// Intrinsic llvm functions
pub const I32_CTPOP: &str = "llvm.ctpop.i32";
pub const I64_CTPOP: &str = "llvm.ctpop.i64";

pub const I32_CLZ: &str = "llvm.ctlz.i32";
pub const I64_CLZ: &str = "llvm.ctlz.i64";

pub const I32_CTZ: &str = "llvm.cttz.i32";
pub const I64_CTZ: &str = "llvm.cttz.i64";

pub const F32_CEIL: &str = "llvm.ceil.f32";
pub const F64_CEIL: &str = "llvm.ceil.f64";

pub const F32_FLOOR: &str = "llvm.floor.f32";
pub const F64_FLOOR: &str = "llvm.floor.f64";

pub const F32_FABS: &str = "llvm.fabs.f32";
pub const F64_FABS: &str = "llvm.fabs.f64";

pub const F32_MAX: &str = "llvm.maxnum.f32";
pub const F64_MAX: &str = "llvm.maxnum.f64";

pub const F32_MIN: &str = "llvm.minnum.f32";
pub const F64_MIN: &str = "llvm.minnum.f64";

pub const F32_SQRT: &str = "llvm.sqrt.f32";
pub const F64_SQRT: &str = "llvm.sqrt.f64";

// TODO: Rewrite this using macros, because this is just gross
pub fn insert_runtime_stubs(opt: &Opt, ctx: &LLVMCtx, m: &LLVMModule) {
    // Initialize region stub, which is a helper function to setup memory
    let initialize_region_type = FunctionType::new(
        <()>::get_type(ctx),
        &[
            <u32>::get_type(ctx),
            <u32>::get_type(ctx),
            PointerType::new(<u8>::get_type(ctx)),
        ],
    );
    m.add_function(INITIALIZE_REGION_STUB, initialize_region_type.to_super());

    // Memory region manipulation stubs
    let memory_size_ty = FunctionType::new(<i32>::get_type(ctx), &[]);
    m.add_function(MEMORY_SIZE, memory_size_ty.to_super());
    let memory_grow_ty = FunctionType::new(<i32>::get_type(ctx), &[<i32>::get_type(ctx)]);
    m.add_function(MEMORY_GROW, memory_grow_ty.to_super());

    // Table interaction function stubs
    let table_add_type = FunctionType::new(
        <()>::get_type(ctx),
        &[
            <u32>::get_type(ctx),
            <u32>::get_type(ctx),
            PointerType::new(<u8>::get_type(ctx)),
        ],
    );
    m.add_function(TABLE_ADD, table_add_type.to_super());

    let table_get_type = FunctionType::new(
        PointerType::new(<u8>::get_type(ctx)),
        &[<u32>::get_type(ctx), <u32>::get_type(ctx)],
    );
    m.add_function(TABLE_FETCH, table_get_type.to_super());

    // Runtime global handling
    if opt.use_runtime_global_handling {
        m.add_function(
            GET_GLOBAL_I32,
            FunctionType::new(<u32>::get_type(ctx), &[<u32>::get_type(ctx)]).to_super(),
        );
        m.add_function(
            SET_GLOBAL_I32,
            FunctionType::new(
                <()>::get_type(ctx),
                &[<u32>::get_type(ctx), <u32>::get_type(ctx)],
            )
            .to_super(),
        );
        m.add_function(
            GET_GLOBAL_I64,
            FunctionType::new(<u64>::get_type(ctx), &[<u32>::get_type(ctx)]).to_super(),
        );
        m.add_function(
            SET_GLOBAL_I64,
            FunctionType::new(
                <()>::get_type(ctx),
                &[<u32>::get_type(ctx), <u64>::get_type(ctx)],
            )
            .to_super(),
        );
    }

    // Rotate left/right types
    let u32_rot_type = FunctionType::new(
        <u32>::get_type(ctx),
        &[<u32>::get_type(ctx), <u32>::get_type(ctx)],
    );
    m.add_function(I32_ROTL, u32_rot_type.to_super());
    m.add_function(I32_ROTR, u32_rot_type.to_super());

    let i64_rot_type = FunctionType::new(
        <u64>::get_type(ctx),
        &[<u64>::get_type(ctx), <u64>::get_type(ctx)],
    );
    m.add_function(I64_ROTL, i64_rot_type.to_super());
    m.add_function(I64_ROTR, i64_rot_type.to_super());

    // Integer to floating point conversions
    let i32_trunc_f32_type = FunctionType::new(<i32>::get_type(ctx), &[<f32>::get_type(ctx)]);
    m.add_function(I32_TRUNC_F32, i32_trunc_f32_type.to_super());
    m.add_function(U32_TRUNC_F32, i32_trunc_f32_type.to_super());

    let i32_trunc_f64_type = FunctionType::new(<i32>::get_type(ctx), &[<f64>::get_type(ctx)]);
    m.add_function(I32_TRUNC_F64, i32_trunc_f64_type.to_super());
    m.add_function(U32_TRUNC_F64, i32_trunc_f64_type.to_super());

    let i64_trunc_f32_type = FunctionType::new(<i64>::get_type(ctx), &[<f32>::get_type(ctx)]);
    m.add_function(I64_TRUNC_F32, i64_trunc_f32_type.to_super());
    m.add_function(U64_TRUNC_F32, i64_trunc_f32_type.to_super());

    let i64_trunc_f64_type = FunctionType::new(<i64>::get_type(ctx), &[<f64>::get_type(ctx)]);
    m.add_function(I64_TRUNC_F64, i64_trunc_f64_type.to_super());
    m.add_function(U64_TRUNC_F64, i64_trunc_f64_type.to_super());

    let f32_trunc_f32_type = FunctionType::new(<f32>::get_type(ctx), &[<f32>::get_type(ctx)]);
    m.add_function(F32_TRUNC_F32, f32_trunc_f32_type.to_super());

    let f64_trunc_f64_type = FunctionType::new(<f64>::get_type(ctx), &[<f64>::get_type(ctx)]);
    m.add_function(F64_TRUNC_F64, f64_trunc_f64_type.to_super());

    // Memory functions
    let get_f32_ty = FunctionType::new(<f32>::get_type(ctx), &[<i32>::get_type(ctx)]);
    m.add_function(GET_F32, get_f32_ty.to_super());
    let set_f32_ty = FunctionType::new(
        <()>::get_type(ctx),
        &[<i32>::get_type(ctx), <f32>::get_type(ctx)],
    );
    m.add_function(SET_F32, set_f32_ty.to_super());

    let get_f64_ty = FunctionType::new(<f64>::get_type(ctx), &[<i32>::get_type(ctx)]);
    m.add_function(GET_F64, get_f64_ty.to_super());
    let set_f64_ty = FunctionType::new(
        <()>::get_type(ctx),
        &[<i32>::get_type(ctx), <f64>::get_type(ctx)],
    );
    m.add_function(SET_F64, set_f64_ty.to_super());

    let get_i8_ty = FunctionType::new(<i8>::get_type(ctx), &[<i32>::get_type(ctx)]);
    m.add_function(GET_I8, get_i8_ty.to_super());
    let set_i8_ty = FunctionType::new(
        <()>::get_type(ctx),
        &[<i32>::get_type(ctx), <i8>::get_type(ctx)],
    );
    m.add_function(SET_I8, set_i8_ty.to_super());

    let get_i16_ty = FunctionType::new(<i16>::get_type(ctx), &[<i32>::get_type(ctx)]);
    m.add_function(GET_I16, get_i16_ty.to_super());
    let set_i16_ty = FunctionType::new(
        <()>::get_type(ctx),
        &[<i32>::get_type(ctx), <i16>::get_type(ctx)],
    );
    m.add_function(SET_I16, set_i16_ty.to_super());

    let get_i32_ty = FunctionType::new(<i32>::get_type(ctx), &[<i32>::get_type(ctx)]);
    m.add_function(GET_I32, get_i32_ty.to_super());
    let set_i32_ty = FunctionType::new(
        <()>::get_type(ctx),
        &[<i32>::get_type(ctx), <i32>::get_type(ctx)],
    );
    m.add_function(SET_I32, set_i32_ty.to_super());

    let get_i64_ty = FunctionType::new(<i64>::get_type(ctx), &[<i32>::get_type(ctx)]);
    m.add_function(GET_I64, get_i64_ty.to_super());
    let set_i64_ty = FunctionType::new(
        <()>::get_type(ctx),
        &[<i32>::get_type(ctx), <i64>::get_type(ctx)],
    );
    m.add_function(SET_I64, set_i64_ty.to_super());

    // LLVM intrinsics
    m.add_function(
        I32_CLZ,
        FunctionType::new(
            <i32>::get_type(ctx),
            &[<i32>::get_type(ctx), <bool>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        I64_CLZ,
        FunctionType::new(
            <i64>::get_type(ctx),
            &[<i64>::get_type(ctx), <bool>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        I32_CTZ,
        FunctionType::new(
            <i32>::get_type(ctx),
            &[<i32>::get_type(ctx), <bool>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        I64_CTZ,
        FunctionType::new(
            <i64>::get_type(ctx),
            &[<i64>::get_type(ctx), <bool>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        I32_CTPOP,
        FunctionType::new(<i32>::get_type(ctx), &[<i32>::get_type(ctx)]).to_super(),
    );
    m.add_function(
        I64_CTPOP,
        FunctionType::new(<i64>::get_type(ctx), &[<i64>::get_type(ctx)]).to_super(),
    );
    m.add_function(
        F32_FABS,
        FunctionType::new(<f32>::get_type(ctx), &[<f32>::get_type(ctx)]).to_super(),
    );
    m.add_function(
        F64_FABS,
        FunctionType::new(<f64>::get_type(ctx), &[<f64>::get_type(ctx)]).to_super(),
    );
    m.add_function(
        F32_SQRT,
        FunctionType::new(<f32>::get_type(ctx), &[<f32>::get_type(ctx)]).to_super(),
    );
    m.add_function(
        F64_SQRT,
        FunctionType::new(<f64>::get_type(ctx), &[<f64>::get_type(ctx)]).to_super(),
    );
    m.add_function(
        F32_MIN,
        FunctionType::new(
            <f32>::get_type(ctx),
            &[<f32>::get_type(ctx), <f32>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        F32_MAX,
        FunctionType::new(
            <f32>::get_type(ctx),
            &[<f32>::get_type(ctx), <f32>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        F32_COPYSIGN,
        FunctionType::new(
            <f32>::get_type(ctx),
            &[<f32>::get_type(ctx), <f32>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        F32_FLOOR,
        FunctionType::new(<f32>::get_type(ctx), &[<f32>::get_type(ctx)]).to_super(),
    );
    m.add_function(
        F32_CEIL,
        FunctionType::new(<f32>::get_type(ctx), &[<f32>::get_type(ctx)]).to_super(),
    );
    m.add_function(
        F32_NEAREST,
        FunctionType::new(<f32>::get_type(ctx), &[<f32>::get_type(ctx)]).to_super(),
    );

    m.add_function(
        F64_MIN,
        FunctionType::new(
            <f64>::get_type(ctx),
            &[<f64>::get_type(ctx), <f64>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        F64_MAX,
        FunctionType::new(
            <f64>::get_type(ctx),
            &[<f64>::get_type(ctx), <f64>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        F64_COPYSIGN,
        FunctionType::new(
            <f64>::get_type(ctx),
            &[<f64>::get_type(ctx), <f64>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        F64_FLOOR,
        FunctionType::new(<f64>::get_type(ctx), &[<f64>::get_type(ctx)]).to_super(),
    );
    m.add_function(
        F64_CEIL,
        FunctionType::new(<f64>::get_type(ctx), &[<f64>::get_type(ctx)]).to_super(),
    );
    m.add_function(
        F64_NEAREST,
        FunctionType::new(<f64>::get_type(ctx), &[<f64>::get_type(ctx)]).to_super(),
    );

    m.add_function(
        U32_DIV,
        FunctionType::new(
            <u32>::get_type(ctx),
            &[<u32>::get_type(ctx), <u32>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        U32_REM,
        FunctionType::new(
            <u32>::get_type(ctx),
            &[<u32>::get_type(ctx), <u32>::get_type(ctx)],
        )
        .to_super(),
    );

    m.add_function(
        I32_DIV,
        FunctionType::new(
            <i32>::get_type(ctx),
            &[<i32>::get_type(ctx), <i32>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        I32_REM,
        FunctionType::new(
            <i32>::get_type(ctx),
            &[<i32>::get_type(ctx), <i32>::get_type(ctx)],
        )
        .to_super(),
    );

    m.add_function(
        U64_DIV,
        FunctionType::new(
            <u64>::get_type(ctx),
            &[<u64>::get_type(ctx), <u64>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        U64_REM,
        FunctionType::new(
            <u64>::get_type(ctx),
            &[<u64>::get_type(ctx), <u64>::get_type(ctx)],
        )
        .to_super(),
    );

    m.add_function(
        I64_DIV,
        FunctionType::new(
            <i64>::get_type(ctx),
            &[<i64>::get_type(ctx), <i64>::get_type(ctx)],
        )
        .to_super(),
    );
    m.add_function(
        I64_REM,
        FunctionType::new(
            <i64>::get_type(ctx),
            &[<i64>::get_type(ctx), <i64>::get_type(ctx)],
        )
        .to_super(),
    );

    m.add_function(
        TRAP_UNREACHABLE,
        FunctionType::new(<()>::get_type(ctx), &[]).to_super(),
    );
}

pub fn get_stub_function<'a>(m_ctx: &'a ModuleCtx, name: &str) -> &'a Function {
    m_ctx
        .llvm_module
        .get_function(name)
        .expect("a stub function name must be in the module!")
}
