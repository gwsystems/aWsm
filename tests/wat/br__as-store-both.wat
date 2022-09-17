(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	(export "memory" (memory 0))

	(func $as-store-both (export "as-store-both") (result i32)
		(block (result i32)
			(i64.store (br 0 (i32.const 32))) (i32.const -1)
		)
	)

    (func (export "_start") (param i32)
		(call $as-store-both (i32.const 32))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
