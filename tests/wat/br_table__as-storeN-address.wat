(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
    (export "memory" (memory 0))

	(func $as-storeN-address (export "as-storeN-address") (result i32)
		(block (result i32)
			(i32.store8 (br_table 0 (i32.const 32) (i32.const 0)) (i32.const 7))
			(i32.const -1)
		)
	)

    (func (export "_start") (param i64)
        (if (i32.ne (call $as-storeN-address) (i32.const 32))(then     
			(call $proc_exit (i32.const 1))
		))

        (call $proc_exit (i32.const 0))
    )
)
