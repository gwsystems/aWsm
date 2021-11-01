(module
    (memory 1)

	(func $type-i32-i32 (export "type-i32-i32") (block (drop (i32.add (br 0)))))

    (func (export "_start") (param i32)
		(call $type-i32-i32)
    )
)
