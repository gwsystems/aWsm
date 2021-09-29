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

(func $shouldNotUse (export "main") (result i32)
    call $secretFunc
    call $magicNumber
    i32.mul
)
)