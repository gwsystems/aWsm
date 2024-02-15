(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
    (export "memory" (memory 0))

	(func $as-store-value (export "as-store-value") (result i32)
		(block (result i32)
			(i64.store (i32.const 2) (br_table 0 (i32.const 31) (i32.const 1)))
			(i32.const -1)
		)
	)

    (func (export "_start") (param i64)
		(if (i32.ne (call $as-store-value) (i32.const 31))(then     
			(call $proc_exit (i32.const 1))
		))
		
        (call $proc_exit (i32.const 0))
    )
)
