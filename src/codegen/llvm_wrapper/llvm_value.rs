use llvm::Compile;
use llvm::Context;
use llvm::Value;

pub struct BasicValue<'a> {
    v: &'a Value,
}

impl<'a> BasicValue<'a> {
    pub fn i32_zero<'b: 'a>(ctx: &'b Context) -> Self {
        Self {
            v: 0i32.compile(ctx),
        }
    }

    pub fn i64_zero<'b: 'a>(ctx: &'b Context) -> Self {
        Self {
            v: 0i64.compile(ctx),
        }
    }

    pub fn f32_zero<'b: 'a>(ctx: &'b Context) -> Self {
        Self {
            v: 0f32.compile(ctx),
        }
    }

    pub fn f64_zero<'b: 'a>(ctx: &'b Context) -> Self {
        Self {
            v: 0f64.compile(ctx),
        }
    }

    pub fn ivalue(&self) -> &'a Value {
        self.v
    }
}
