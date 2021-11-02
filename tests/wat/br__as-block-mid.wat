(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)

	(func $dummy)
	
	(func $as-block-mid (export "as-block-mid")
		(block (call $dummy) (br 0) (call $dummy))
	)

    (func (export "_start") (param i32)
		(call $as-block-mid)

        (call $proc_exit (i32.const 0))
    )
)
