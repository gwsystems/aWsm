(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
    (export "memory" (memory 0))

	(func $as-select-first (export "as-select-first") (param i32 i32) (result i32)
		(block (result i32)
			(select
				(br_table 0 (i32.const 5) (i32.const 0)) 
				(local.get 0) 
				(local.get 1)
			)
		)
	)

    (func (export "_start") (param i64)
        (if (i32.ne (call $as-select-first (i32.const 0) (i32.const 6)) (i32.const 5))(then     
			(call $proc_exit (i32.const 1))
		))

        (if (i32.ne (call $as-select-first (i32.const 1) (i32.const 6)) (i32.const 5))(then     
			(call $proc_exit (i32.const 2))
		))

        (call $proc_exit (i32.const 0))
    )
)
