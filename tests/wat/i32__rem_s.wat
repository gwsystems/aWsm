;; i32 operations

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
    (export "memory" (memory 0))

	(func $rem_s (export "rem_s") (param $x i32) (param $y i32) (result i32) (i32.rem_s (local.get $x) (local.get $y)))

	(func $_start (export "_start")
		;; (assert_trap (invoke "rem_s" (i32.const 1) (i32.const 0)) "integer divide by zero")
		;; (assert_trap (invoke "rem_s" (i32.const 0) (i32.const 0)) "integer divide by zero")
		(if (i32.ne (call $rem_s (i32.const 0x7fffffff) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const 1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const 0) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const 0) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const -1) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const 0x80000000) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const 0x80000000) (i32.const 2)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const 0x80000001) (i32.const 1000)) (i32.const -647)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const 5) (i32.const 2)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const -5) (i32.const 2)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const 5) (i32.const -2)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const -5) (i32.const -2)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const 7) (i32.const 3)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const -7) (i32.const 3)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const 7) (i32.const -3)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const -7) (i32.const -3)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const 11) (i32.const 5)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $rem_s (i32.const 17) (i32.const 7)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 1))
		))

	)
)

;; Type check

;; (assert_invalid (module (func (result i32) (i32.rem_s (i64.const 0) (f32.const 0)))) "type mismatch")
