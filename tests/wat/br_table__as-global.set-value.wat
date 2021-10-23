(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)

	(global $a (mut i32) (i32.const 10))
	
	(func $as-global.set-value (export "as-global.set-value") (result i32)
		(block (result i32)
			(global.set $a (br_table 0 (i32.const 1) (i32.const 1)))
			(i32.const -1)
		)
	)

    (func (export "_start") (param i64)
		(if (i32.ne (call $as-global.set-value) (i32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

        (call $proc_exit (i32.const 0))
    )
)
