(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	(export "memory" (memory 0))

	(func $f (param i32 i32 i32) (result i32) (i32.const -1))

	(type $sig (func (param i32 i32 i32) (result i32)))

	(table funcref (elem $f))

	(func $as-call_indirect-first (export "as-call_indirect-first") (result i32)
		(block (result i32)
			(call_indirect (type $sig)
				(i32.const 0)
				(br 0 (i32.const 21)) (i32.const 2) (i32.const 3)
			)
		)
	)

    (func (export "_start") (param i32)
		(call $as-call_indirect-first (i32.const 21))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
