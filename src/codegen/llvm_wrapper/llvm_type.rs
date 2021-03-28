use llvm::Compile;
use llvm::Type;

#[derive(Clone)]
pub struct BasicType<'a> {
    ty: &'a Type,
}

impl<'a> BasicType<'a> {
    pub fn void_ty<'b: 'a>(ctx: &'b llvm::Context) -> Self {
        Self {
            ty: <()>::get_type(ctx),
        }
    }

    pub fn i32_ty<'b: 'a>(ctx: &'b llvm::Context) -> Self {
        Self {
            ty: <i32>::get_type(ctx),
        }
    }

    pub fn i64_ty<'b: 'a>(ctx: &'b llvm::Context) -> Self {
        Self {
            ty: <i64>::get_type(ctx),
        }
    }

    pub fn f32_ty<'b: 'a>(ctx: &'b llvm::Context) -> Self {
        Self {
            ty: <f32>::get_type(ctx),
        }
    }

    pub fn f64_ty<'b: 'a>(ctx: &'b llvm::Context) -> Self {
        Self {
            ty: <f64>::get_type(ctx),
        }
    }

    pub fn itype(&self) -> &'a Type {
        self.ty
    }
}

#[derive(Clone)]
pub struct FunctionType<'a> {
    ty: &'a Type,
}

impl<'a> FunctionType<'a> {
    pub fn new<'b: 'a, 'c: 'a>(
        _ctx: &'b llvm::Context,
        params: &[BasicType<'c>],
        res: BasicType<'c>,
    ) -> Self {
        let mut wrapped_params = Vec::new();
        wrapped_params.extend(params.iter().map(BasicType::itype));

        let ty = llvm::FunctionType::new(res.itype(), &wrapped_params);

        Self { ty }
    }

    #[allow(dead_code)]
    pub fn new_ret_void<'b: 'a, 'c: 'a>(ctx: &'b llvm::Context, params: &[BasicType<'c>]) -> Self {
        let mut wrapped_params = Vec::new();
        wrapped_params.extend(params.iter().map(BasicType::itype));

        let ty = llvm::FunctionType::new(BasicType::void_ty(ctx).itype(), &wrapped_params);

        Self { ty }
    }

    pub fn itype(&self) -> &'a Type {
        self.ty
    }
}
