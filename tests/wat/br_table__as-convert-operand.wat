(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
    (export "memory" (memory 0))

	(func $as-convert-operand (export "as-convert-operand") (result i32)
		(block (result i32)
			(i32.wrap_i64 (br_table 0 (i32.const 41) (i32.const 0)))
		)
	)

    (func (export "_start") (param i32)
		(if (i32.ne (call $as-convert-operand) (i32.const 41))(then     
			(call $proc_exit (i32.const 1))
		))

        (call $proc_exit (i32.const 0))
    )
)
