(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
    (export "memory" (memory 0))

	(func $as-select-second (export "as-select-second") (param i32 i32) (result i32)
		(block (result i32)
			(select
				(local.get 0) 
				(br_table 0 (i32.const 6) (i32.const 1)) 
				(local.get 1)
			)
		)
	)

    (func (export "_start") (param i64)
		(if (i32.ne (call $as-select-second (i32.const 0) (i32.const 6)) (i32.const 6))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-select-second (i32.const 1) (i32.const 6)) (i32.const 6))(then     
			(call $proc_exit (i32.const 2))
		))

        (call $proc_exit (i32.const 0))
    )
)
