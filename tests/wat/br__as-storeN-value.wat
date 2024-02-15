(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	(export "memory" (memory 0))

	(func $as-storeN-value (export "as-storeN-value") (result i32)
		(block (result i32)
			(i64.store16 (i32.const 2) (br 0 (i32.const 33))) (i32.const -1)
		)
	)

    (func (export "_start") (param i32)
		(call $as-storeN-value (i32.const 33))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
