(module

(memory 2)
(export "memory" (memory 0))
(table $tbl 0 anyfunc)

(func $secretFunc (result i32)
    i32.const 5
)

(func $magicNumber (result i32)
    i32.const 42
)

(func $mult (param $first i32) (param $second i32) (result i32)
    local.get 0
    local.get 1
    i32.mul
)

(func $partial (param $first i32) (param i32) (result i32)
    local.get 0
    local.get 1
    i32.mul
)

(func $shouldNotUse (export "main") (result i32)
    call $secretFunc
    call $magicNumber
    call $mult
)
)
