(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	(export "memory" (memory 0))

	(func $as-memory.grow-size (export "as-memory.grow-size") (result i32)
		(block (result i32) (memory.grow (br 0 (i32.const 40))))
	)

    (func (export "_start") (param i32)
		(call $as-memory.grow-size (i32.const 40))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
