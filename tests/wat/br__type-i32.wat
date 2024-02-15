(module
    (memory 1)
    (export "memory" (memory 0))

	(func $type-i32 (export "type-i32") 
        (block (drop (i32.ctz (br 0))))
    )

    (func (export "_start") (param i32)
		(call $type-i32)
    )
)
