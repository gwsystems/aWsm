(module
    (memory 1)

	(func $type-f64-f64 (export "type-f64-f64") (block (drop (f64.add (br 0)))))

    (func (export "_start") (param i32)
		(call $type-f64-f64)
    )
)
