use std::collections::{HashMap, HashSet};
use std::error::Error;
use std::str;

use wasmparser::ImportSectionEntryType;
use wasmparser::MemoryType;
use wasmparser::Operator;
use wasmparser::Parser;
use wasmparser::ParserState;
use wasmparser::SectionCode;
use wasmparser::TableType;
use wasmparser::Type;
use wasmparser::WasmDecoder;
use wasmparser::{CustomSectionKind, Name, NameSectionReader, Naming, TypeOrFuncType};
use wasmparser::{ElemSectionEntryTable, ElementItem, ExternalKind};
use wasmparser::{FuncType, TypeDef};

#[derive(Debug)]
pub struct FunctionNameMap {
    name: String,
    locals: HashMap<u32, String>,
}

#[derive(Debug)]
pub struct WasmModule {
    pub source_name: String,
    name_counter: u64,

    pub types: Vec<FuncType>,
    pub globals: Vec<Global>,

    // The indices of the elements in functions are used as the keys in function_name_maps
    pub functions: Vec<Function>,
    pub function_name_maps: HashMap<u32, FunctionNameMap>,
    pub function_names: HashSet<String>,
    pub start_function: Option<u32>,

    pub memories: Vec<MemoryType>,
    pub data_initializers: Vec<DataInitializer>,

    pub tables: Vec<TableType>,
    pub table_initializers: Vec<TableInitializer>,

    pub exports: Vec<Export>,
}

#[derive(Clone, Debug)]
pub enum Global {
    Imported {
        name: String,
        content_type: Type,
        mutable: bool,
    },
    InModule {
        generated_name: String,
        content_type: Type,
        mutable: bool,
        initializer: Vec<Instruction>,
    },
}

impl Global {
    pub fn set_name(&mut self, new_name: String) {
        *self = match *self {
            Global::Imported { .. } => panic!("Cannot remap the name of an import!"),
            Global::InModule {
                content_type,
                mutable,
                ref initializer,
                ..
            } => Global::InModule {
                content_type,
                mutable,
                initializer: initializer.clone(),
                generated_name: new_name,
            },
        }
    }

    pub fn in_memory_size(&self) -> usize {
        let typ = match self {
            Global::Imported { content_type, .. } => content_type,
            Global::InModule { content_type, .. } => content_type,
        };

        match typ {
            Type::I32 => 4,
            Type::I64 => 8,
            Type::F32 => 4,
            Type::F64 => 8,
            Type::V128 => 16,
            Type::FuncRef => 4,
            Type::ExternRef => 8,
            Type::Func => 4,
            Type::EmptyBlockType => 0,
        }
    }
}

#[derive(Clone, Debug)]
pub enum Function {
    Imported {
        source: String,
        name: String,
        appended: String,
        ty_index: u32,
        ty: FuncType,
    },
    Declared {
        ty_index: u32,
        ty: FuncType,
    },
    Implemented {
        f: ImplementedFunction,
    },
}

impl Function {
    fn declared_but_unimplemented(&self) -> Option<(FuncType, u32)> {
        if let Function::Declared { ty, ty_index } = self {
            Some((ty.clone(), *ty_index))
        } else {
            None
        }
    }

    pub fn count_args(&self) -> usize {
        match self {
            Function::Imported { ty, .. } => ty.params.len(),
            Function::Declared { .. } => {
                panic!("Malformed wasm, a function was declared but not implemented")
            }
            Function::Implemented { ref f } => f.ty.as_ref().unwrap().params.len(),
        }
    }

    pub fn get_name(&self) -> &str {
        match self {
            Function::Imported { appended, .. } => &appended,
            Function::Declared { .. } => {
                panic!("Malformed wasm, a function was declared but not implemented")
            }
            Function::Implemented { f } => &f.generated_name,
        }
    }

    pub fn set_name(&mut self, new_name: String) {
        *self = match *self {
            Function::Imported { .. } => panic!("Cannot set the name of an imported function"),
            Function::Declared { .. } => {
                panic!("Malformed wasm, a function was declared but not implemented")
            }
            Function::Implemented { ref mut f } => {
                f.generated_name = new_name;
                Function::Implemented { f: f.clone() }
            }
        }
    }

    pub fn set_locals_name_map(&mut self, locals_name_map: HashMap<u32, String>) {
        *self = match *self {
            Function::Imported { .. } => {
                panic!("Cannot set the local name of an imported function")
            }
            Function::Declared { .. } => {
                panic!("Malformed wasm, a function was declared but not implemented")
            }
            Function::Implemented { ref mut f } => {
                f.locals_name_map = locals_name_map;
                Function::Implemented { f: f.clone() }
            }
        }
    }

    pub fn get_type(&self) -> &FuncType {
        match self {
            Function::Imported { ty, .. } => &ty,
            Function::Declared { .. } => {
                panic!("Malformed wasm, a function was declared but not implemented")
            }
            Function::Implemented { ref f } => f.get_type(),
        }
    }

    pub fn get_type_index(&self) -> u32 {
        match self {
            Function::Imported { ty_index, .. } => *ty_index,
            Function::Declared { .. } => {
                panic!("Malformed wasm, a function was declared but not implemented")
            }
            Function::Implemented { ref f } => f.ty_index.unwrap(),
        }
    }

    pub fn has_return(&self) -> bool {
        match self {
            Function::Imported { ty, .. } => !ty.returns.is_empty(),
            Function::Declared { .. } => {
                panic!("Malformed wasm, a function was declared but not implemented")
            }
            Function::Implemented { f } => f.has_return(),
        }
    }
}

#[derive(Debug)]
pub enum Export {
    Memory { name: String, index: usize },
    Function { name: String, index: usize },
    Global { name: String, index: usize },
}

#[derive(Clone, Debug)]
pub struct ImplementedFunction {
    pub generated_name: String,
    pub ty: Option<FuncType>,
    pub ty_index: Option<u32>,
    pub locals: Vec<Type>,
    pub locals_name_map: HashMap<u32, String>,
    pub code: Vec<Instruction>,
}

impl ImplementedFunction {
    pub fn has_return(&self) -> bool {
        match self.ty {
            Some(ref ty) => !ty.returns.is_empty(),
            None => panic!("Malformed wasm, a function has no type"),
        }
    }

    pub fn get_type(&self) -> &FuncType {
        match self.ty {
            Some(ref ty) => &ty,
            None => panic!("Malformed wasm, a function has no type"),
        }
    }

    pub fn get_return_type(&self) -> Vec<Type> {
        match self.ty {
            Some(ref ty) => {
                if ty.returns.len() == 1 {
                    let mut res = Vec::new();
                    res.push(ty.returns[0]);
                    res
                } else if ty.returns.is_empty() {
                    Vec::new()
                } else {
                    let mut res: Vec<Type> = ty.returns.iter().map(|e| *e).collect();
                    res
                    // panic!("Malformed wasm, a function has too many return types")
                }
            }
            None => panic!("Malformed wasm, a function has no type"),
        }
    }
}

#[derive(Debug)]
pub struct DataInitializer {
    pub offset_expression: Vec<Instruction>,
    pub body: Vec<Vec<u8>>,
}

#[derive(Debug)]
pub struct TableInitializer {
    pub offset_expression: Vec<Instruction>,
    pub function_indexes: Vec<u32>,
}

#[derive(Clone, Debug, PartialEq)]
#[allow(clippy::upper_case_acronyms)]
pub enum Instruction {
    BlockStart { produced_type: Vec<TypeOrFuncType> },
    LoopStart { produced_type: Vec<TypeOrFuncType> },
    If { produced_type: Vec<TypeOrFuncType> },
    Else,
    End,

    Br { depth: u32 },
    BrIf { depth: u32 },
    BrTable { table: Vec<u32>, default: u32 },

    Return,
    Unreachable,

    Call { index: u32 },
    CallIndirect { type_index: u32 },
    Drop,
    Nop,
    Select,

    LocalGet { index: u32 },
    LocalSet { index: u32 },
    LocalTee { index: u32 },

    GlobalGet { index: u32 },
    GlobalSet { index: u32 },

    I32Const(i32),

    I32WrapI64,

    I32ReinterpretF32,

    I32TruncF32S,
    I32TruncF32U,
    I32TruncF64S,
    I32TruncF64U,

    I32Add,
    I32And,
    I32Clz,
    I32Ctz,
    I32DivS,
    I32DivU,
    I32Mul,
    I32Or,
    I32Popcnt,
    I32RemS,
    I32RemU,
    I32Rotl,
    I32Rotr,
    I32Shl,
    I32ShrS,
    I32ShrU,
    I32Sub,
    I32Xor,

    I32Eqz,
    I32Eq,
    I32Ne,
    I32LeS,
    I32LeU,
    I32LtS,
    I32LtU,
    I32GeS,
    I32GeU,
    I32GtS,
    I32GtU,

    I64Const(i64),

    I64ExtendI32S,
    I64ExtendI32U,

    I64ReinterpretF64,

    I64TruncF32S,
    I64TruncF32U,
    I64TruncF64S,
    I64TruncF64U,

    I64Add,
    I64And,
    I64Clz,
    I64Ctz,
    I64DivS,
    I64DivU,
    I64Mul,
    I64Or,
    I64Popcnt,
    I64RemS,
    I64RemU,
    I64Rotl,
    I64Rotr,
    I64Shl,
    I64ShrS,
    I64ShrU,
    I64Sub,
    I64Xor,

    I64Eqz,
    I64Eq,
    I64Ne,
    I64LeS,
    I64LeU,
    I64LtS,
    I64LtU,
    I64GeS,
    I64GeU,
    I64GtS,
    I64GtU,

    F32Const(f32),

    F32DemoteF64,

    F32ReinterpretI32,

    F32ConvertI32S,
    F32ConvertI32U,
    F32ConvertI64S,
    F32ConvertI64U,

    F32Abs,
    F32Add,
    F32Div,
    F32Mul,
    F32Neg,
    F32Sub,
    F32Sqrt,
    F32Trunc,

    F32Eq,
    F32Ne,
    F32Le,
    F32Lt,
    F32Ge,
    F32Gt,

    F32Min,
    F32Max,
    F32Copysign,
    F32Floor,
    F32Ceil,
    F32Nearest,

    F64Const(f64),

    F64PromoteF32,

    F64ReinterpretI64,

    F64ConvertI32S,
    F64ConvertI32U,
    F64ConvertI64S,
    F64ConvertI64U,

    F64Abs,
    F64Add,
    F64Div,
    F64Mul,
    F64Neg,
    F64Sub,
    F64Sqrt,
    F64Trunc,

    F64Eq,
    F64Ne,
    F64Le,
    F64Lt,
    F64Ge,
    F64Gt,

    F64Min,
    F64Max,
    F64Copysign,
    F64Floor,
    F64Ceil,
    F64Nearest,

    I32Load { flags: u32, offset: u32 },
    I32Store { flags: u32, offset: u32 },
    I32Load8S { flags: u32, offset: u32 },
    I32Load8U { flags: u32, offset: u32 },
    I32Store8 { flags: u32, offset: u32 },
    I32Load16S { flags: u32, offset: u32 },
    I32Load16U { flags: u32, offset: u32 },
    I32Store16 { flags: u32, offset: u32 },

    I64Load { flags: u32, offset: u32 },
    I64Store { flags: u32, offset: u32 },
    I64Load8S { flags: u32, offset: u32 },
    I64Load8U { flags: u32, offset: u32 },
    I64Store8 { flags: u32, offset: u32 },
    I64Load16S { flags: u32, offset: u32 },
    I64Load16U { flags: u32, offset: u32 },
    I64Store16 { flags: u32, offset: u32 },
    I64Load32S { flags: u32, offset: u32 },
    I64Load32U { flags: u32, offset: u32 },
    I64Store32 { flags: u32, offset: u32 },

    F32Load { flags: u32, offset: u32 },
    F32Store { flags: u32, offset: u32 },

    F64Load { flags: u32, offset: u32 },
    F64Store { flags: u32, offset: u32 },

    MemorySize,
    MemoryGrow,
}

impl<'a> From<&'a Operator<'a>> for Instruction {
    fn from(o: &Operator<'a>) -> Self {
        match *o {
            // FIXME: Does this `produced_type` logic even match the spec? Do we need to introspect func types?
            // (suggested fix -- blacklist func types here)
            Operator::Block { ty } => {
                let produced_type = if ty == TypeOrFuncType::Type(Type::EmptyBlockType) {
                    None
                } else {
                    Some(ty)
                };
                Instruction::BlockStart { produced_type }
            }
            Operator::Loop { ty } => {
                let produced_type = if ty == TypeOrFuncType::Type(Type::EmptyBlockType) {
                    new Vec()
                } else {
                    Some(ty)
                };
                Instruction::LoopStart { produced_type }
            }
            Operator::If { ty } => {
                let produced_type = if ty == TypeOrFuncType::Type(Type::EmptyBlockType) {
                    Vec::new()
                } else {
                    ty
                };
                Instruction::If { produced_type }
            }
            Operator::Else => Instruction::Else,
            Operator::End => Instruction::End,

            Operator::Br { relative_depth } => Instruction::Br {
                depth: relative_depth,
            },
            Operator::BrIf { relative_depth } => Instruction::BrIf {
                depth: relative_depth,
            },
            Operator::BrTable { ref table } => {
                let (table, default) = table
                    .read_table()
                    .expect("well formed wasm must have good tables");
                Instruction::BrTable {
                    table: table.to_vec(),
                    default,
                }
            }

            Operator::Return => Instruction::Return,
            Operator::Unreachable => Instruction::Unreachable,

            Operator::Call { function_index } => Instruction::Call {
                index: function_index,
            },
            Operator::CallIndirect { index, table_index } => {
                assert_eq!(table_index, 0);
                Instruction::CallIndirect { type_index: index }
            }
            Operator::Drop => Instruction::Drop,
            Operator::Nop => Instruction::Nop,
            Operator::Select => Instruction::Select,

            Operator::LocalGet { local_index } => Instruction::LocalGet { index: local_index },
            Operator::LocalSet { local_index } => Instruction::LocalSet { index: local_index },
            Operator::LocalTee { local_index } => Instruction::LocalTee { index: local_index },

            Operator::GlobalGet { global_index } => Instruction::GlobalGet {
                index: global_index,
            },
            Operator::GlobalSet { global_index } => Instruction::GlobalSet {
                index: global_index,
            },

            Operator::I32Const { value } => Instruction::I32Const(value),

            Operator::I32WrapI64 => Instruction::I32WrapI64,

            Operator::I32ReinterpretF32 => Instruction::I32ReinterpretF32,

            Operator::I32TruncF32S => Instruction::I32TruncF32S,
            Operator::I32TruncF32U => Instruction::I32TruncF32U,
            Operator::I32TruncF64S => Instruction::I32TruncF64S,
            Operator::I32TruncF64U => Instruction::I32TruncF64U,

            Operator::I32Add => Instruction::I32Add,
            Operator::I32And => Instruction::I32And,
            Operator::I32Clz => Instruction::I32Clz,
            Operator::I32Ctz => Instruction::I32Ctz,
            Operator::I32DivS => Instruction::I32DivS,
            Operator::I32DivU => Instruction::I32DivU,
            Operator::I32Mul => Instruction::I32Mul,
            Operator::I32Or => Instruction::I32Or,
            Operator::I32Popcnt => Instruction::I32Popcnt,
            Operator::I32RemS => Instruction::I32RemS,
            Operator::I32RemU => Instruction::I32RemU,
            Operator::I32Rotl => Instruction::I32Rotl,
            Operator::I32Rotr => Instruction::I32Rotr,
            Operator::I32Shl => Instruction::I32Shl,
            Operator::I32ShrS => Instruction::I32ShrS,
            Operator::I32ShrU => Instruction::I32ShrU,
            Operator::I32Sub => Instruction::I32Sub,
            Operator::I32Xor => Instruction::I32Xor,

            Operator::I32Eqz => Instruction::I32Eqz,
            Operator::I32Eq => Instruction::I32Eq,
            Operator::I32Ne => Instruction::I32Ne,
            Operator::I32LeS => Instruction::I32LeS,
            Operator::I32LeU => Instruction::I32LeU,
            Operator::I32LtS => Instruction::I32LtS,
            Operator::I32LtU => Instruction::I32LtU,
            Operator::I32GeS => Instruction::I32GeS,
            Operator::I32GeU => Instruction::I32GeU,
            Operator::I32GtS => Instruction::I32GtS,
            Operator::I32GtU => Instruction::I32GtU,

            Operator::I64Const { value } => Instruction::I64Const(value),

            Operator::I64ExtendI32S => Instruction::I64ExtendI32S,
            Operator::I64ExtendI32U => Instruction::I64ExtendI32U,

            Operator::I64ReinterpretF64 => Instruction::I64ReinterpretF64,

            Operator::I64TruncF32S => Instruction::I64TruncF32S,
            Operator::I64TruncF32U => Instruction::I64TruncF32U,
            Operator::I64TruncF64S => Instruction::I64TruncF64S,
            Operator::I64TruncF64U => Instruction::I64TruncF64U,

            Operator::I64Add => Instruction::I64Add,
            Operator::I64And => Instruction::I64And,
            Operator::I64Clz => Instruction::I64Clz,
            Operator::I64Ctz => Instruction::I64Ctz,
            Operator::I64DivS => Instruction::I64DivS,
            Operator::I64DivU => Instruction::I64DivU,
            Operator::I64Mul => Instruction::I64Mul,
            Operator::I64Or => Instruction::I64Or,
            Operator::I64Popcnt => Instruction::I64Popcnt,
            Operator::I64RemS => Instruction::I64RemS,
            Operator::I64RemU => Instruction::I64RemU,
            Operator::I64Rotl => Instruction::I64Rotl,
            Operator::I64Rotr => Instruction::I64Rotr,
            Operator::I64Shl => Instruction::I64Shl,
            Operator::I64ShrS => Instruction::I64ShrS,
            Operator::I64ShrU => Instruction::I64ShrU,
            Operator::I64Sub => Instruction::I64Sub,
            Operator::I64Xor => Instruction::I64Xor,

            Operator::I64Eqz => Instruction::I64Eqz,
            Operator::I64Eq => Instruction::I64Eq,
            Operator::I64Ne => Instruction::I64Ne,
            Operator::I64LeS => Instruction::I64LeS,
            Operator::I64LeU => Instruction::I64LeU,
            Operator::I64LtS => Instruction::I64LtS,
            Operator::I64LtU => Instruction::I64LtU,
            Operator::I64GeS => Instruction::I64GeS,
            Operator::I64GeU => Instruction::I64GeU,
            Operator::I64GtS => Instruction::I64GtS,
            Operator::I64GtU => Instruction::I64GtU,

            Operator::F32Const { value } => {
                let v: f32 = f32::from_bits(value.bits());
                Instruction::F32Const(v)
            }

            Operator::F32DemoteF64 => Instruction::F32DemoteF64,

            Operator::F32ReinterpretI32 => Instruction::F32ReinterpretI32,

            Operator::F32ConvertI32S => Instruction::F32ConvertI32S,
            Operator::F32ConvertI32U => Instruction::F32ConvertI32U,
            Operator::F32ConvertI64S => Instruction::F32ConvertI64S,
            Operator::F32ConvertI64U => Instruction::F32ConvertI64U,

            Operator::F32Abs => Instruction::F32Abs,
            Operator::F32Add => Instruction::F32Add,
            Operator::F32Div => Instruction::F32Div,
            Operator::F32Mul => Instruction::F32Mul,
            Operator::F32Neg => Instruction::F32Neg,
            Operator::F32Sub => Instruction::F32Sub,
            Operator::F32Sqrt => Instruction::F32Sqrt,
            Operator::F32Trunc => Instruction::F32Trunc,

            Operator::F32Eq => Instruction::F32Eq,
            Operator::F32Ne => Instruction::F32Ne,
            Operator::F32Le => Instruction::F32Le,
            Operator::F32Lt => Instruction::F32Lt,
            Operator::F32Ge => Instruction::F32Ge,
            Operator::F32Gt => Instruction::F32Gt,

            Operator::F32Min => Instruction::F32Min,
            Operator::F32Max => Instruction::F32Max,
            Operator::F32Copysign => Instruction::F32Copysign,
            Operator::F32Floor => Instruction::F32Floor,
            Operator::F32Ceil => Instruction::F32Ceil,
            Operator::F32Nearest => Instruction::F32Nearest,

            Operator::F64Const { value } => {
                let v: f64 = f64::from_bits(value.bits());
                Instruction::F64Const(v)
            }

            Operator::F64PromoteF32 => Instruction::F64PromoteF32,

            Operator::F64ReinterpretI64 => Instruction::F64ReinterpretI64,

            Operator::F64ConvertI32S => Instruction::F64ConvertI32S,
            Operator::F64ConvertI32U => Instruction::F64ConvertI32U,
            Operator::F64ConvertI64S => Instruction::F64ConvertI64S,
            Operator::F64ConvertI64U => Instruction::F64ConvertI64U,

            Operator::F64Abs => Instruction::F64Abs,
            Operator::F64Add => Instruction::F64Add,
            Operator::F64Div => Instruction::F64Div,
            Operator::F64Mul => Instruction::F64Mul,
            Operator::F64Neg => Instruction::F64Neg,
            Operator::F64Sub => Instruction::F64Sub,
            Operator::F64Sqrt => Instruction::F64Sqrt,
            Operator::F64Trunc => Instruction::F64Trunc,

            Operator::F64Eq => Instruction::F64Eq,
            Operator::F64Ne => Instruction::F64Ne,
            Operator::F64Le => Instruction::F64Le,
            Operator::F64Lt => Instruction::F64Lt,
            Operator::F64Ge => Instruction::F64Ge,
            Operator::F64Gt => Instruction::F64Gt,

            Operator::F64Min => Instruction::F64Min,
            Operator::F64Max => Instruction::F64Max,
            Operator::F64Copysign => Instruction::F64Copysign,
            Operator::F64Floor => Instruction::F64Floor,
            Operator::F64Ceil => Instruction::F64Ceil,
            Operator::F64Nearest => Instruction::F64Nearest,

            Operator::I32Load { ref memarg } => Instruction::I32Load {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I32Store { ref memarg } => Instruction::I32Store {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I32Load8S { ref memarg } => Instruction::I32Load8S {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I32Load8U { ref memarg } => Instruction::I32Load8U {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I32Store8 { ref memarg } => Instruction::I32Store8 {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I32Load16S { ref memarg } => Instruction::I32Load16S {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I32Load16U { ref memarg } => Instruction::I32Load16U {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I32Store16 { ref memarg } => Instruction::I32Store16 {
                flags: memarg.flags,
                offset: memarg.offset,
            },

            Operator::I64Load { ref memarg } => Instruction::I64Load {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I64Store { ref memarg } => Instruction::I64Store {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I64Load8S { ref memarg } => Instruction::I64Load8S {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I64Load8U { ref memarg } => Instruction::I64Load8U {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I64Store8 { ref memarg } => Instruction::I64Store8 {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I64Load16S { ref memarg } => Instruction::I64Load16S {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I64Load16U { ref memarg } => Instruction::I64Load16U {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I64Store16 { ref memarg } => Instruction::I64Store16 {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I64Load32S { ref memarg } => Instruction::I64Load32S {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I64Load32U { ref memarg } => Instruction::I64Load32U {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::I64Store32 { ref memarg } => Instruction::I64Store32 {
                flags: memarg.flags,
                offset: memarg.offset,
            },

            Operator::F32Load { ref memarg } => Instruction::F32Load {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::F32Store { ref memarg } => Instruction::F32Store {
                flags: memarg.flags,
                offset: memarg.offset,
            },

            Operator::F64Load { ref memarg } => Instruction::F64Load {
                flags: memarg.flags,
                offset: memarg.offset,
            },
            Operator::F64Store { ref memarg } => Instruction::F64Store {
                flags: memarg.flags,
                offset: memarg.offset,
            },

            Operator::MemorySize { .. } => Instruction::MemorySize,
            Operator::MemoryGrow { .. } => Instruction::MemoryGrow,

            ref e => unimplemented!("{:?}", e),
        }
    }
}

enum ProcessState {
    Outer,
    TypeSection,
    ImportSection,
    FunctionSection,
    TableSection,
    MemorySection,
    ExportSection,
    CodeSection,
    FunctionCode(ImplementedFunction),

    DataSection,
    DataSectionEntry {
        memory_id: u32,
        offset_expression: Option<Vec<Instruction>>,
        body: Option<Vec<Vec<u8>>>,
    },
    DataOffsetExpression {
        memory_id: u32,
    },
    DataSectionBody {
        memory_id: u32,
        offset_expression: Vec<Instruction>,
    },
    StartSection,
    TableElementSection,
    TableElementEntry {
        table_id: u32,
    },
    TableOffsetExpression {
        table_id: u32,
    },
    TableEntryBody {
        table_id: u32,
        offset_expression: Vec<Instruction>,
    },

    GlobalSection,
    GlobalSectionEntry {
        content_type: Type,
        mutable: bool,
    },

    CustomSection {
        name: Vec<u8>,
        kind: CustomSectionKind,
    },

    Finished,
}

impl WasmModule {
    fn new(input_filename: &str) -> WasmModule {
        WasmModule {
            source_name: input_filename.to_string(),
            name_counter: 0,
            types: Vec::new(),
            globals: Vec::new(),
            functions: Vec::new(),
            start_function: None,
            tables: Vec::new(),
            table_initializers: Vec::new(),
            memories: Vec::new(),
            data_initializers: Vec::new(),
            exports: Vec::new(),
            function_name_maps: HashMap::new(),
            function_names: HashSet::new(),
        }
    }

    pub fn from_wasm_parser(input_filename: &str, p: &mut Parser) -> WasmModule {
        let mut m = WasmModule::new(input_filename);
        m.process_wasm(p);
        // Fix Up Names if Optional Names section is present
        // This assumes that the functions are ordered exactly as in WebAssembly, as
        // the namespace maps from an integral index to a string.
        if m.function_name_maps.len() > 0 {
            for (i, func) in m.functions.iter_mut().enumerate() {
                if let Function::Implemented { f: _ } = func {
                    if let Some(function_name_map) = m.function_name_maps.get(&(i as u32)) {
                        func.set_name("wasmf_internal_".to_string() + &function_name_map.name);

                        func.set_locals_name_map(function_name_map.locals.clone());
                    }
                }
            }
        }
        m
    }

    pub fn log_diagnostics(&self) {
        let global_variable_memory_use: usize =
            self.globals.iter().map(|g| g.in_memory_size()).sum();
        info!("Globals taking up {} bytes", global_variable_memory_use);

        let mut data_initializer_memory_use = 0;
        for initializer in &self.data_initializers {
            for body_bytes in &initializer.body {
                data_initializer_memory_use += body_bytes.len();
            }
        }
        info!(
            "Data initializers taking up {} bytes",
            data_initializer_memory_use
        );

        let mut function_table_entries = 0;
        for initializer in &self.table_initializers {
            function_table_entries += initializer.function_indexes.len();
        }
        info!(
            "Function table entries {} (ignoring fragmentation)",
            function_table_entries
        );
    }

    fn implement_function(&mut self, mut f: ImplementedFunction) {
        for existing_function in &mut self.functions {
            if let Some((ty, ty_index)) = existing_function.declared_but_unimplemented() {
                f.ty = Some(ty);
                f.ty_index = Some(ty_index);
                *existing_function = Function::Implemented { f };
                return;
            }
        }
        panic!(
            "Malformed wasm -- attempt to implement a function when no declaration was available"
        )
    }

    fn generate_function_name(&mut self) -> String {
        let result = format!("wasmf_internal_{}", self.name_counter);
        self.name_counter += 1;
        result
    }

    fn generate_global_name(&mut self) -> String {
        let result = format!("wasmg_internal_{}", self.name_counter);
        self.name_counter += 1;
        result
    }

    // What follows is 400 lines of code to load the data from the WASM parsing into this data structure
    // I would avoid messing with it if possible
    fn process_outer_section(&mut self, p: &mut Parser) -> ProcessState {
        match p.read() {
            &ParserState::BeginWasm { .. } => ProcessState::Outer,
            &ParserState::BeginSection { code, .. } => match code {
                SectionCode::Type => ProcessState::TypeSection,
                SectionCode::Import => ProcessState::ImportSection,
                SectionCode::Function => ProcessState::FunctionSection,
                SectionCode::Table => ProcessState::TableSection,
                SectionCode::Memory => ProcessState::MemorySection,
                SectionCode::Export => ProcessState::ExportSection,
                SectionCode::Code => ProcessState::CodeSection,
                SectionCode::Data => ProcessState::DataSection,
                SectionCode::Element => ProcessState::TableElementSection,
                SectionCode::Global => ProcessState::GlobalSection,
                SectionCode::Start => ProcessState::StartSection,
                SectionCode::Custom { name, kind } => ProcessState::CustomSection {
                    name: Vec::from(name.as_bytes()),
                    kind,
                },
                e => panic!("Have not implemented section code {:?}", e),
            },
            &ParserState::EndWasm => ProcessState::Finished,
            e => panic!("Have not implemented outer section state {:?}", e),
        }
    }

    /// Parses the optional name section, which provides human-friendly names
    /// of functions and locals as a debugging aid, and saves to an in-memory
    /// HashMap.
    fn process_name_section(&mut self, data: &[u8]) -> Result<(), Box<dyn Error>> {
        let reader = NameSectionReader::new(data, 0)?;
        for name in reader {
            match name? {
                Name::Function(nm) => {
                    let mut map = nm.get_map()?;
                    for _ in 0..map.get_count() {
                        let Naming { name, index } = map.read()?;

                        // The function names in a WebAssembly Name Section are not
                        // guaranteed to be unique. Duplicates occur when compiling
                        // from languages with function overloading. In order to
                        // guarantee unique symbols, we append a unique suffix.
                        let mut suffix = 0;
                        let mut unique_name = name.to_string();
                        while self.function_names.contains(&unique_name) {
                            suffix = suffix + 1;
                            unique_name = format!("{}_{}", name, suffix);
                        }

                        self.function_names.insert(unique_name.clone());
                        self.function_name_maps.insert(
                            index,
                            FunctionNameMap {
                                name: unique_name,
                                locals: HashMap::new(),
                            },
                        );
                    }
                }
                Name::Local(nm) => {
                    let mut fn_reader = nm.get_function_local_reader()?;
                    for _ in 0..fn_reader.get_count() {
                        let fn_locals = fn_reader.read()?;
                        let mut fn_locals_map = fn_locals.get_map()?;
                        for _ in 0..fn_locals_map.get_count() {
                            let local = fn_locals_map.read()?;
                            if let Some(function_name_map) =
                                self.function_name_maps.get_mut(&fn_locals.func_index)
                            {
                                function_name_map
                                    .locals
                                    .insert(local.index, String::from(local.name));
                            }
                        }
                    }
                }
                Name::Module(module_name) => {
                    // This is not derived from a file name.
                    // Extracts the optional label from the module instruction
                    // (module $moduleName).
                    if let Ok(name) = module_name.get_name() {
                        info!("Module Name: {}", name);
                    }
                }
            }
        }
        Ok(())
    }

    fn process_custom_section(
        &mut self,
        p: &mut Parser,
        _: Vec<u8>,
        kind: CustomSectionKind,
    ) -> ProcessState {
        loop {
            match p.read() {
                &ParserState::SectionRawData(data) => {
                    match kind {
                        CustomSectionKind::Name => {
                            if let Err(err) = self.process_name_section(data) {
                                error!("Error processing name section: {}", err);
                            };
                        }
                        CustomSectionKind::Unknown => {
                            info!("Skipping Unknown Custom Section");
                        }
                        CustomSectionKind::Producers => {
                            // https://github.com/WebAssembly/tool-conventions/blob/main/ProducersSection.md
                            info!("Skipping Producers Custom Section");
                        }
                        CustomSectionKind::SourceMappingURL => {
                            // https://github.com/WebAssembly/tool-conventions/blob/main/Debugging.md
                            info!("Skipping Source Mapping URL Custom Section");
                        }
                        CustomSectionKind::Reloc => {
                            // https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md#relocation-sections
                            info!("Skipping Relocation Custom Section");
                        }
                        CustomSectionKind::Linking => {
                            // https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md
                            info!("Skipping Linking Custom Section");
                        }
                    }
                }
                &ParserState::EndSection => return ProcessState::Outer,
                e => panic!("Custom Section Parsing Error {:?}", e),
            }
        }
    }

    fn process_start_section(&mut self, p: &mut Parser) -> ProcessState {
        match p.read() {
            &ParserState::StartSectionEntry(start_fn_idx) => {
                self.start_function = Some(start_fn_idx);
                ProcessState::StartSection
            }
            &ParserState::EndSection => ProcessState::Outer,
            e => panic!("Have not implemented type section state {:?}", e),
        }
    }

    fn process_type_section(&mut self, p: &mut Parser) -> ProcessState {
        match p.read() {
            &ParserState::TypeSectionEntry(ref f) => {
                match f {
                    TypeDef::Func(func_type) => self.types.push(func_type.clone()),
                    TypeDef::Instance(_instance_type) => {
                        panic!(
                            "TypeDef::Instance (type {:?}) not implemented",
                            _instance_type
                        )
                    }
                    TypeDef::Module(_module_type) => {
                        panic!("TypeDef::Module (type {:?}) not implemented", _module_type)
                    }
                }
                ProcessState::TypeSection
            }
            &ParserState::EndSection => ProcessState::Outer,
            e => panic!("Have not implemented type section state {:?}", e),
        }
    }

    fn process_memory_section(&mut self, p: &mut Parser) -> ProcessState {
        match p.read() {
            &ParserState::MemorySectionEntry(mt) => {
                self.memories.push(mt);
                ProcessState::MemorySection
            }
            &ParserState::EndSection => ProcessState::Outer,
            e => panic!("Have not implemented memory section state {:?}", e),
        }
    }

    fn process_import_section(&mut self, p: &mut Parser) -> ProcessState {
        match p.read() {
            &ParserState::ImportSectionEntry {
                module,
                field,
                ref ty,
            } => {
                match ty {
                    ImportSectionEntryType::Function(i) => {
                        let source = module.to_string();
                        let name = field
                            .expect("Expected imported function to have name")
                            .to_string();
                        let appended = source.clone() + "_" + &name;
                        self.functions.push(Function::Imported {
                            source,
                            name,
                            appended,
                            ty: self.types[*i as usize].clone(),
                            ty_index: *i,
                        });
                    }
                    ImportSectionEntryType::Global(global_ty) => {
                        let source = module.to_string();
                        let name = field
                            .expect("Expected imported global to have name")
                            .to_string();
                        let appended = source + "_" + &name;

                        self.globals.push(Global::Imported {
                            name: appended,
                            content_type: global_ty.content_type,
                            mutable: global_ty.mutable,
                        });
                    }
                    ImportSectionEntryType::Memory(memory_ty) => {
                        self.memories.push(*memory_ty);
                    }
                    ImportSectionEntryType::Table(table_ty) => {
                        self.tables.push(*table_ty);
                    }
                    ImportSectionEntryType::Module(i) => {
                        panic!("ImportSectionEntryType::Module (idx {}) not implemented", i)
                    }
                    ImportSectionEntryType::Instance(i) => {
                        panic!(
                            "ImportSectionEntryType::Instance (idx {}) not implemented",
                            i
                        )
                    }
                }
                ProcessState::ImportSection
            }
            &ParserState::EndSection => ProcessState::Outer,
            e => panic!("Have not implemented import section state {:?}", e),
        }
    }

    /// Adds the function declarations from the WebAssembly module to the functions
    /// vector. This maintains the same ordering at the source WebAssembly module,
    /// which ensures that the indices of the vector match the indices used in the
    /// name map of the custom Names section.
    fn process_function_section(&mut self, p: &mut Parser) -> ProcessState {
        match p.read() {
            &ParserState::FunctionSectionEntry(i) => {
                self.functions.push(Function::Declared {
                    ty: self.types[i as usize].clone(),
                    ty_index: i,
                });
                ProcessState::FunctionSection
            }
            &ParserState::EndSection => ProcessState::Outer,
            e => panic!("Have not implemented function section state {:?}", e),
        }
    }

    fn process_table_section(&mut self, p: &mut Parser) -> ProcessState {
        match p.read() {
            &ParserState::TableSectionEntry(tt) => {
                self.tables.push(tt);
                ProcessState::TableSection
            }
            &ParserState::EndSection => ProcessState::Outer,
            e => panic!("Have not implemented table section state {:?}", e),
        }
    }

    fn process_export_section(&mut self, p: &mut Parser) -> ProcessState {
        match p.read() {
            &ParserState::ExportSectionEntry { field, kind, index } => {
                let name = field.to_string();
                let export = match kind {
                    ExternalKind::Function => Export::Function {
                        name,
                        index: index as usize,
                    },
                    ExternalKind::Memory => Export::Memory {
                        name,
                        index: index as usize,
                    },
                    ExternalKind::Global => Export::Global {
                        name,
                        index: index as usize,
                    },
                    e => panic!("Have not implemented export kind {:?}", e),
                };

                self.exports.push(export);
                ProcessState::ExportSection
            }
            &ParserState::EndSection => ProcessState::Outer,
            e => panic!("Have not implemented export section state {:?}", e),
        }
    }

    fn process_code_section(&mut self, p: &mut Parser) -> ProcessState {
        match p.read() {
            &ParserState::BeginFunctionBody { .. } => {
                ProcessState::FunctionCode(ImplementedFunction {
                    generated_name: self.generate_function_name(),
                    ty: None,
                    ty_index: None,
                    locals: Vec::new(),
                    locals_name_map: HashMap::new(),
                    code: Vec::new(),
                })
            }
            &ParserState::EndSection => ProcessState::Outer,
            e => panic!("Have not implemented code section state {:?}", e),
        }
    }

    fn process_function_code(
        &mut self,
        p: &mut Parser,
        mut f: ImplementedFunction,
    ) -> ProcessState {
        match p.read() {
            &ParserState::FunctionBodyLocals { ref locals } => {
                for (i, ty) in locals.iter() {
                    for _ in 0..*i {
                        f.locals.push(*ty);
                    }
                }
                ProcessState::FunctionCode(f)
            }
            &ParserState::CodeOperator(ref o) => {
                f.code.push(o.into());
                ProcessState::FunctionCode(f)
            }
            &ParserState::EndFunctionBody => {
                self.implement_function(f);
                ProcessState::CodeSection
            }
            e => panic!("Have not implemented function code state {:?}", e),
        }
    }

    fn process_data_section(&mut self, p: &mut Parser) -> ProcessState {
        match p.read() {
            &ParserState::BeginActiveDataSectionEntry(i) => ProcessState::DataSectionEntry {
                memory_id: i,
                offset_expression: None,
                body: None,
            },
            &ParserState::EndSection => ProcessState::Outer,
            e => panic!("Have not implemented data section state {:?}", e),
        }
    }

    fn process_data_section_entry(
        &mut self,
        p: &mut Parser,
        memory_id: u32,
        offset_expression: Option<Vec<Instruction>>,
        body: Option<Vec<Vec<u8>>>,
    ) -> ProcessState {
        match p.read() {
            &ParserState::BeginInitExpressionBody => {
                ProcessState::DataOffsetExpression { memory_id }
            }
            // The ignored field here stores the size of the entry -- but this is implicit in the body vec we build anyway
            &ParserState::BeginDataSectionEntryBody(_) => ProcessState::DataSectionBody {
                memory_id,
                offset_expression: offset_expression
                    .expect("A data section entry body must be preceded by an offset expression!"),
            },
            &ParserState::EndDataSectionEntry => {
                self.data_initializers.push(DataInitializer {
                    offset_expression: offset_expression
                        .expect("An initializer must have an offset expression!"),
                    body: body.expect("A data section entry must have a body"),
                });
                ProcessState::DataSection
            }
            e => panic!("Have not implemented data section entry state {:?}", e),
        }
    }

    fn process_offset_expression(&mut self, p: &mut Parser, memory_id: u32) -> ProcessState {
        let mut code = Vec::new();
        loop {
            match p.read() {
                &ParserState::InitExpressionOperator(ref o) => {
                    code.push(o.into());
                }
                &ParserState::EndInitExpressionBody => {
                    return ProcessState::DataSectionEntry {
                        memory_id,
                        offset_expression: Some(code),
                        body: None,
                    };
                }
                e => panic!("Have not implemented offset expression state {:?}", e),
            }
        }
    }

    fn process_data_section_body(
        &mut self,
        p: &mut Parser,
        memory_id: u32,
        offset_expression: Vec<Instruction>,
    ) -> ProcessState {
        let mut body: Vec<Vec<u8>> = Vec::new();
        loop {
            match p.read() {
                &ParserState::DataSectionEntryBodyChunk(d) => body.push(d.to_vec()),
                &ParserState::EndDataSectionEntryBody => {
                    return ProcessState::DataSectionEntry {
                        memory_id,
                        offset_expression: Some(offset_expression),
                        body: Some(body),
                    };
                }
                e => panic!("Have not implemented data section body state {:?}", e),
            }
        }
    }

    fn process_table_element_section(&mut self, p: &mut Parser) -> ProcessState {
        match p.read() {
            &ParserState::BeginElementSectionEntry { table, ty: _ } => match table {
                ElemSectionEntryTable::Active(table_id) => {
                    ProcessState::TableElementEntry { table_id }
                }
                ElemSectionEntryTable::Passive => {
                    panic!("ElemSectionEntryTable::Passive not implemented")
                }
                ElemSectionEntryTable::Declared => {
                    panic!("ElemSectionEntryTable::Declared not implemented")
                }
            },
            &ParserState::EndSection => ProcessState::Outer,
            e => panic!("Have not implemented table element section state {:?}", e),
        }
    }

    fn process_table_entry(&mut self, p: &mut Parser, table_id: u32) -> ProcessState {
        match p.read() {
            &ParserState::BeginInitExpressionBody => {
                ProcessState::TableOffsetExpression { table_id }
            }
            e => panic!("Have not implemented table entry state {:?}", e),
        }
    }

    fn process_table_offset_expression(&mut self, p: &mut Parser, table_id: u32) -> ProcessState {
        let mut code = Vec::new();
        loop {
            match p.read() {
                &ParserState::InitExpressionOperator(ref o) => {
                    code.push(o.into());
                }
                &ParserState::EndInitExpressionBody => {
                    return ProcessState::TableEntryBody {
                        table_id,
                        offset_expression: code,
                    };
                }
                e => panic!("Have not implemented offset expression state {:?}", e),
            }
        }
    }

    fn process_table_entry_body(
        &mut self,
        p: &mut Parser,
        table_id: u32,
        offset_expression: Vec<Instruction>,
    ) -> ProcessState {
        let mut function_indexes: Vec<u32> = Vec::new();
        loop {
            match p.read() {
                &ParserState::ElementSectionEntryBody(ref v) => {
                    for elem in v.iter() {
                        match elem {
                            ElementItem::Func(idx) => function_indexes.push(*idx),
                            ElementItem::Null(ty) => {
                                panic!("ElementItem::Null (type {:?}) not implemented", ty)
                            }
                        }
                    }
                }
                &ParserState::EndElementSectionEntry => {
                    assert_eq!(table_id, 0);
                    let ti = TableInitializer {
                        offset_expression,
                        function_indexes,
                    };
                    self.table_initializers.push(ti);
                    return ProcessState::TableElementSection;
                }
                e => panic!("Have not implemented table entry body state {:?}", e),
            }
        }
    }

    fn process_table_global_section(&mut self, p: &mut Parser) -> ProcessState {
        match p.read() {
            &ParserState::BeginGlobalSectionEntry(gt) => ProcessState::GlobalSectionEntry {
                content_type: gt.content_type,
                mutable: gt.mutable,
            },
            &ParserState::EndSection => ProcessState::Outer,
            e => panic!("Have not implemented global section state {:?}", e),
        }
    }

    fn process_global_section_entry(
        &mut self,
        p: &mut Parser,
        content_type: Type,
        mutable: bool,
    ) -> ProcessState {
        match p.read() {
            &ParserState::BeginInitExpressionBody => {}
            e => panic!(
                "Have not implemented global section entry first state {:?}",
                e
            ),
        }

        let mut code = Vec::new();
        loop {
            match p.read() {
                &ParserState::InitExpressionOperator(ref o) => {
                    code.push(o.into());
                }
                &ParserState::EndInitExpressionBody => break,
                e => panic!(
                    "Have not implemented initialization expression state {:?}",
                    e
                ),
            }
        }

        match p.read() {
            &ParserState::EndGlobalSectionEntry => {}
            e => panic!(
                "Have not implemented global section entry tail state {:?}",
                e
            ),
        }

        let generated_name = self.generate_global_name();

        self.globals.push(Global::InModule {
            content_type,
            mutable,
            initializer: code,
            generated_name,
        });

        ProcessState::GlobalSection
    }

    fn process_wasm(&mut self, p: &mut Parser) {
        let mut s = ProcessState::Outer;
        loop {
            s = match s {
                ProcessState::Outer => self.process_outer_section(p),
                ProcessState::StartSection => self.process_start_section(p),
                ProcessState::TypeSection => self.process_type_section(p),
                ProcessState::ImportSection => self.process_import_section(p),
                ProcessState::FunctionSection => self.process_function_section(p),
                ProcessState::TableSection => self.process_table_section(p),
                ProcessState::MemorySection => self.process_memory_section(p),
                ProcessState::ExportSection => self.process_export_section(p),
                ProcessState::CodeSection => self.process_code_section(p),
                ProcessState::FunctionCode(f) => self.process_function_code(p, f),
                ProcessState::DataSection => self.process_data_section(p),
                ProcessState::DataSectionEntry {
                    memory_id,
                    offset_expression,
                    body,
                } => self.process_data_section_entry(p, memory_id, offset_expression, body),
                ProcessState::DataOffsetExpression { memory_id } => {
                    self.process_offset_expression(p, memory_id)
                }
                ProcessState::DataSectionBody {
                    memory_id,
                    offset_expression,
                } => self.process_data_section_body(p, memory_id, offset_expression),
                ProcessState::TableElementSection => self.process_table_element_section(p),
                ProcessState::TableElementEntry { table_id } => {
                    self.process_table_entry(p, table_id)
                }
                ProcessState::TableOffsetExpression { table_id } => {
                    self.process_table_offset_expression(p, table_id)
                }
                ProcessState::TableEntryBody {
                    table_id,
                    offset_expression,
                } => self.process_table_entry_body(p, table_id, offset_expression),
                ProcessState::GlobalSection => self.process_table_global_section(p),
                ProcessState::GlobalSectionEntry {
                    content_type,
                    mutable,
                } => self.process_global_section_entry(p, content_type, mutable),

                ProcessState::CustomSection { name, kind } => {
                    self.process_custom_section(p, name, kind)
                }

                ProcessState::Finished => break,
            };
        }
    }
}
