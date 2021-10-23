(module
    (memory 1)

	(func $type-f32 (export "type-f32")
		(block (drop (f32.neg (br_table 0 0 (i32.const 0)))))
	)

    (func (export "_start") (param i32)
        (call $type-f32)
    )
)
