(module
    (memory 1)
    (export "memory" (memory 0))

	(func $as-select-cond (export "as-select-cond") (result i32)
		(block (result i32)
			(select (i32.const 0) (i32.const 1) (br 0 (i32.const 7)))
		)
	)

    (func (export "_start") (param i32)
		(call $as-select-cond (i32.const 7))
		drop
		drop
    )
)
