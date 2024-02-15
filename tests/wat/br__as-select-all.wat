(module
    (memory 1)
    (export "memory" (memory 0))

	(func $as-select-all (export "as-select-all") (result i32)
		(block (result i32) (select (br 0 (i32.const 8))))
	)

    (func (export "_start") (param i32)
		(call $as-select-all (i32.const 8))
		drop
		drop
    )
)
