(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	(export "memory" (memory 0))

	(func $f (param i32 i32 i32) (result i32) (i32.const -1))

	(func $as-call-last (export "as-call-last") (result i32)
		(block (result i32)
			(call $f (i32.const 1) (i32.const 2) (br 0 (i32.const 14)))
		)
	)
    
	(func (export "_start") (param i32)
		(call $as-call-last (i32.const 14))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
