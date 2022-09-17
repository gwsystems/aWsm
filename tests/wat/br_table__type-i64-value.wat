(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
    (export "memory" (memory 0))

	(func $type-i64-value (export "type-i64-value") (result i64)
		(block (result i64) (i64.ctz (br_table 0 0 (i64.const 2) (i32.const 0))))
	)

    (func (export "_start") (param i64)
        (if (i64.ne (call $type-i64-value) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

        (call $proc_exit (i32.const 0))
    )
)
