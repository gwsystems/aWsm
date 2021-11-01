;; i64 operations

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
    (export "memory" (memory 0))

	(func $rem_s (export "rem_s") (param $x i64) (param $y i64) (result i64) (i64.rem_s (local.get $x) (local.get $y)))

	(func $_start (export "_start")
		;; (assert_trap (invoke "rem_s" (i64.const 1) (i64.const 0)) "integer divide by zero")
		;; (assert_trap (invoke "rem_s" (i64.const 0) (i64.const 0)) "integer divide by zero")

		(if (i64.ne (call $rem_s (i64.const 0x7fffffffffffffff) (i64.const -1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const 1) (i64.const 1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const 0) (i64.const 1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const 0) (i64.const -1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const -1) (i64.const -1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const 0x8000000000000000) (i64.const -1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const 0x8000000000000000) (i64.const 2)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const 0x8000000000000001) (i64.const 1000)) (i64.const -807)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const 5) (i64.const 2)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const -5) (i64.const 2)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const 5) (i64.const -2)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const -5) (i64.const -2)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const 7) (i64.const 3)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const -7) (i64.const 3)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const 7) (i64.const -3)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const -7) (i64.const -3)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const 11) (i64.const 5)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_s (i64.const 17) (i64.const 7)) (i64.const 3)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(call $proc_exit (i32.const 0))
	)
)
