(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)

    (func $dummy)

	(func $as-loop-last (export "as-loop-last") (result i32)
		(loop (result i32)
			(nop) (call $dummy) (br_table 1 1 1 (i32.const 5) (i32.const 1))
		)
	)

    (func (export "_start") (param i64)
		(if (i32.ne (call $as-loop-last) (i32.const 5)) (then     
			(call $proc_exit (i32.const 1))
		))

        (call $proc_exit (i32.const 0))
    )
)
