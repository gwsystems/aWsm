(module
    (memory 1)

	(func $type-i64-i64 (export "type-i64-i64") (block (drop (i64.add (br 0)))))

    (func (export "_start") (param i32)
		(call $type-i64-i64)
    )
)
