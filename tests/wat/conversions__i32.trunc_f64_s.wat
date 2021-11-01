(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)

	(func $i32.trunc_f64_s (export "i32.trunc_f64_s") (param $x f64) (result i32) (i32.trunc_f64_s (local.get $x)))

	(func $_start (export "_start")
		(if (i32.ne (call $i32.trunc_f64_s (f64.const 0.0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 26))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const -0.0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 27))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const 0x0.0000000000001p-1022)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 28))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const -0x0.0000000000001p-1022)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 29))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const 1.0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 30))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const 0x1.199999999999ap+0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 31))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const 1.5)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 32))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const -1.0)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 33))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const -0x1.199999999999ap+0)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 34))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const -1.5)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 35))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const -1.9)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 36))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const -2.0)) (i32.const -2)) (then     
			(call $proc_exit (i32.const 37))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const 2147483647.0)) (i32.const 2147483647)) (then     
			(call $proc_exit (i32.const 38))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const -2147483648.0)) (i32.const -2147483648)) (then     
			(call $proc_exit (i32.const 39))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const -2147483648.9)) (i32.const -2147483648)) (then     
			(call $proc_exit (i32.const 40))
		))

		(if (i32.ne (call $i32.trunc_f64_s (f64.const  2147483647.9)) (i32.const  2147483647)) (then     
			(call $proc_exit (i32.const 41))
		))

		;; (assert_trap (invoke "i32.trunc_f64_s" (f64.const 2147483648.0)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f64_s" (f64.const -2147483649.0)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f64_s" (f64.const inf)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f64_s" (f64.const -inf)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f64_s" (f64.const nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i32.trunc_f64_s" (f64.const nan:0x4000000000000)) "invalid conversion to integer")
		;; (assert_trap (invoke "i32.trunc_f64_s" (f64.const -nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i32.trunc_f64_s" (f64.const -nan:0x4000000000000)) "invalid conversion to integer")

		(call $proc_exit (i32.const 0))
	)	
)

