(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
    (export "memory" (memory 0))

	(func $as-loadN-address (export "as-loadN-address") (result i64)
		(block (result i64) (i64.load8_s (br_table 0 (i64.const 30) (i32.const 1))))
	)

    (func (export "_start") (param i64)
        (if (i64.ne (call $as-loadN-address) (i64.const 30))(then     
			(call $proc_exit (i32.const 1))
		))

        (call $proc_exit (i32.const 0))
    )
)
