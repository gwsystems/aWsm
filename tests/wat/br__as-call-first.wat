(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	
	(func $f (param i32 i32 i32) (result i32) (i32.const -1))

	(func $as-call-first (export "as-call-first") (result i32)
		(block (result i32)
			(call $f (br 0 (i32.const 12)) (i32.const 2) (i32.const 3))
		)
	)

    (func (export "_start") (param i32)
		(call $as-call-first (i32.const 12))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
