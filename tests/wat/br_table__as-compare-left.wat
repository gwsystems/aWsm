(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
    (export "memory" (memory 0))

	(func $as-compare-left (export "as-compare-left") (result i32)
		(block (result i32)
			(f64.le (br_table 0 0 (i32.const 43) (i32.const 0)) (f64.const 10))
		)
	)

    (func (export "_start") (param i32)
		(if (i32.ne (call $as-compare-left) (i32.const 43))(then     
			(call $proc_exit (i32.const 1))
		))

        (call $proc_exit (i32.const 0))
    )
)
