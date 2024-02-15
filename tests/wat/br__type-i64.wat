(module
	(memory 1)
    (export "memory" (memory 0))

	(func $type-i64 (export "type-i64") 
		(block (drop (i64.ctz (br 0))))
	)

	(func (export "_start") (param i32)
		(call $type-i64)
	)
)
