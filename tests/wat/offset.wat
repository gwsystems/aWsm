(module $coolModule

(memory 2)
(export "memory" (memory 0))
(table $tbl 0 anyfunc)

(func $shouldNotUse (export "_start") (result i32)
    (i32.store offset=42 (i32.const 0) (i32.const 101))
    (i32.load (i32.const 42))
    return
)
)
