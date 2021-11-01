(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	
	(func $as-binary-both (export "as-binary-both") (result i32)
		(block (result i32) (i32.add (br 0 (i32.const 46))))
	)

    (func (export "_start") (param i32)
		(call $as-binary-both (i32.const 46))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
