(module
    (memory 1)
    (export "memory" (memory 0))

	(func $type-f32-f32 (export "type-f32-f32") (block (drop (f32.add (br 0)))))

    (func (export "_start") (param i32)
		(call $type-f32-f32)
    )
)
