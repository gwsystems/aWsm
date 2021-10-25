(module
	(memory 1)

	(func $type-i64-value (export "type-i64-value") (result i64)
		(block (result i64) (i64.ctz (br 0 (i64.const 2))))
	)

	(func (export "_start") (param i32)
		(call $type-i64-value (i64.const 2))
		drop
		drop
	)
)
