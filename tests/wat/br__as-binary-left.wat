(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	(export "memory" (memory 0))

	(func $as-binary-left (export "as-binary-left") (result i32)
		(block (result i32) (i32.add (br 0 (i32.const 3)) (i32.const 10)))
	)

    (func (export "_start") (param i32)
		(call $as-binary-left (i32.const 3))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
