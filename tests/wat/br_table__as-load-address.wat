(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)

	(func $as-load-address (export "as-load-address") (result f32)
		(block (result f32) (f32.load (br_table 0 (f32.const 1.7) (i32.const 1))))
	)

    (func (export "_start") (param i64)
		(if (f32.ne (call $as-load-address) (f32.const 1.7))(then     
			(call $proc_exit (i32.const 1))
		))

        (call $proc_exit (i32.const 0))
    )
)
