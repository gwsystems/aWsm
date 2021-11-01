(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)

	(func $type-f32-value (export "type-f32-value") (result f32)
		(block (result f32) (f32.neg (br_table 0 0 (f32.const 3) (i32.const 0))))
	)

    (func (export "_start") (param i32)
		(if (f32.ne (call $type-f32-value) (f32.const 3)) (then     
			(call $proc_exit (i32.const 3))
		))

        (call $proc_exit (i32.const 0))
    )
)
