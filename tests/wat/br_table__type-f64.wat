(module
    (memory 1)

	(func $type-f64 (export "type-f64")
		(block (drop (f64.neg (br_table 0 0 (i32.const 0)))))
	)

    (func (export "_start") (param i32)
        (call $type-f64)
    )
)
