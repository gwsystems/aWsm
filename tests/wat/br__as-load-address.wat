(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	(export "memory" (memory 0))

	(func $as-load-address (export "as-load-address") (result f32)
		(block (result f32) (f32.load (br 0 (f32.const 1.7))))
	)

    (func (export "_start") (param i32)
		(call $as-load-address (f32.const 1.7))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
