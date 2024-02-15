(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
    (export "memory" (memory 0))

	(func $as-compare-right (export "as-compare-right") (result i32)
		(block (result i32)
			(f32.ne (f32.const 10) (br_table 0 (i32.const 42) (i32.const 0)))
		)
	)

    (func (export "_start") (param i32)
		(if (i32.ne (call $as-compare-right) (i32.const 42))(then     
			(call $proc_exit (i32.const 1))
		))

        (call $proc_exit (i32.const 0))
    )
)
