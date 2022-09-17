(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	(export "memory" (memory 0))

	(func $as-storeN-address (export "as-storeN-address") (result i32)
		(block (result i32)
			(i32.store8 (br 0 (i32.const 32)) (i32.const 7)) (i32.const -1)
		)
	)

    (func (export "_start") (param i32)
		(call $as-storeN-address (i32.const 32))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
