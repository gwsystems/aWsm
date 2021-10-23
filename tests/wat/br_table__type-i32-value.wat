(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)

	(func $type-i32-value (export "type-i32-value") (result i32)
		(block (result i32) (i32.ctz (br_table 0 0 (i32.const 1) (i32.const 0))))
	)

    (func (export "_start") (param i32)
        (if (i32.ne (call $type-i32-value) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

        (call $proc_exit (i32.const 0))
    )
)
