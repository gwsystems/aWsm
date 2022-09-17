(module
    (memory 1)
    (export "memory" (memory 0))

    (func $dummy)

	(func $as-block-first (export "as-block-first")
		(block (br_table 0 0 0 (i32.const 0)) (call $dummy))
	)

    (func (export "_start") (param i64)
        (call $as-block-first)
    )
)
