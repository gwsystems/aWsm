(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
    (export "memory" (memory 0))

	(func $as-if-cond (export "as-if-cond") (result i32)
		(block (result i32)
			(if (result i32)
				(br_table 0 (i32.const 2) (i32.const 0))
			(then (i32.const 0))
			(else (i32.const 1))
			)
		)
	)

    (func (export "_start") (param i64)
        (if (i32.ne (call $as-if-cond) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

        (call $proc_exit (i32.const 0))
    )
)
