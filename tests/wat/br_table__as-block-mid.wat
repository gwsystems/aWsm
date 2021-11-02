(module
    (memory 1)

    (func $dummy)

	(func $as-block-mid (export "as-block-mid")
		(block (call $dummy) (br_table 0 0 0 (i32.const 0)) (call $dummy))
	)

    (func (export "_start") (param i64)
        (call $as-block-mid)
    )
)
