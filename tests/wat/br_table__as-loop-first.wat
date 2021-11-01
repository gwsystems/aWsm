(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)

	(func $as-loop-first (export "as-loop-first") (result i32)
		(loop (result i32) 
            (br_table 1 1 (i32.const 3) (i32.const 0)) 
            (i32.const 1)
        )
	)

    (func (export "_start") (param i64)
        (if (i32.ne (call $as-loop-first) (i32.const 3)) (then     
			(call $proc_exit (i32.const 1))
		))

        (call $proc_exit (i32.const 0))
    )
)
