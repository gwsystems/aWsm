(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	
	(func $as-convert-operand (export "as-convert-operand") (result i32)
		(block (result i32) (i32.wrap_i64 (br 0 (i32.const 41))))
	)

    (func (export "_start") (param i32)
		(call $as-convert-operand (i32.const 41))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
