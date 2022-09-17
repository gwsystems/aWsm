(module
	(memory 1)
    (export "memory" (memory 0))

	(func $type-i32-value (export "type-i32-value") (result i32)
		(block (result i32) (i32.ctz (br 0 (i32.const 1))))
	)

	(func (export "_start") (param i32)
		(call $type-i32-value (i32.const 1))
		drop
		drop
	)
)
