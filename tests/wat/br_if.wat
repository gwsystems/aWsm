(module $coolModule

(memory 2)
(export "memory" (memory 0))
(table $tbl 0 anyfunc)

(func $shouldNotUse (export "_start") (result i32)
    (block
        (block
            (block
                (i32.eq (i32.const 0) (i32.const 1))
                br_if 1
                (i32.eq (i32.const 1) (i32.const 1))
                br_if 2
                ;; (br 0)
                (return (i32.const 42))
            )
            (return (i32.const 1))
        )
        (return (i32.const 2))
    )
    (return (i32.const 3))
)
)
