(module
    (memory 1)

	(func $type-f32 (export "type-f32") (block (drop (f32.neg (br 0)))))

    (func (export "_start") (param i32)
		(call $type-f32)
    )
)
