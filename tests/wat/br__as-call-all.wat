(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	(export "memory" (memory 0))

	(func $f (param i32 i32 i32) (result i32) (i32.const -1))

	(func $as-call-all (export "as-call-all") (result i32)
		(block (result i32) (call $f (br 0 (i32.const 15))))
	)

    (func (export "_start") (param i32)
		(call $as-call-all (i32.const 15))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
