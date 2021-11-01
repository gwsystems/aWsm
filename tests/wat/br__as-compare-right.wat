(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)

	(func $as-compare-right (export "as-compare-right") (result i32)
		(block (result i32) (f32.ne (f32.const 10) (br 0 (i32.const 42))))
	)

    (func (export "_start") (param i32)
		(call $as-compare-right (i32.const 42))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
