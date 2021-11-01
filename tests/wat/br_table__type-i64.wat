(module
    (memory 1)

	(func $type-i64 (export "type-i64")
		(block (drop (i64.ctz (br_table 0 0 (i32.const 0)))))
	)

    (func (export "_start") (param i64)
        (call $type-i64)
    )
)
