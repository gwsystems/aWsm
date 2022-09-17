(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	(export "memory" (memory 0))

	(func $as-store-address (export "as-store-address") (result i32)
		(block (result i32)
			(f64.store (br 0 (i32.const 30)) (f64.const 7)) (i32.const -1)
		)
	)

    (func (export "_start") (param i32)
		(call $as-store-address (i32.const 30))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
