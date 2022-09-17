;; Test `if` operator

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))

	;; Auxiliary definition
	(memory 1)
    (export "memory" (memory 0))

	(func $dummy)

	(func $as-unary-operand (export "as-unary-operand") (param i32) (result i32)
		(i32.ctz
			(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 13))
				(else (call $dummy) (i32.const -13))
			)
		)
	)
	
	(func $_start (export "_start")
		(if (i32.ne (call $as-unary-operand (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-unary-operand (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 2))
		))

		(if (i32.ne (call $as-unary-operand (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 3))
		))

		(call $proc_exit (i32.const 0))
	)
)
