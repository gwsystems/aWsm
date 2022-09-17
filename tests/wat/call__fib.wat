(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
    (export "memory" (memory 0))

	(func $fib (export "fib") (param i64) (result i64)
		(if (result i64) (i64.le_u (local.get 0) (i64.const 1))
			(then (i64.const 1))
			(else
				(i64.add
					(call $fib (i64.sub (local.get 0) (i64.const 2)))
					(call $fib (i64.sub (local.get 0) (i64.const 1)))
				)
			)
		)
	)

    (func (export "_start") (param i64)
		(if (i64.ne (call $fib (i64.const 0)) (i64.const 1)) (then
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $fib (i64.const 1)) (i64.const 1)) (then
			(call $proc_exit (i32.const 2))
		))

		(if (i64.ne (call $fib (i64.const 2)) (i64.const 2)) (then
			(call $proc_exit (i32.const 3))
		))

		(if (i64.ne (call $fib (i64.const 5)) (i64.const 8)) (then
			(call $proc_exit (i32.const 4))
		))

		(if (i64.ne (call $fib (i64.const 20)) (i64.const 10946)) (then
			(call $proc_exit (i32.const 5))
		))
		
        (call $proc_exit (i32.const 0))
    )
)
