(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)

	(func $i32.trunc_f64_u (export "i32.trunc_f64_u") (param $x f64) (result i32) (i32.trunc_f64_u (local.get $x)))

	(func $_start (export "_start")
		(if (i32.ne (call $i32.trunc_f64_u (f64.const 0.0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i32.ne (call $i32.trunc_f64_u (f64.const -0.0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 42))
		))

		(if (i32.ne (call $i32.trunc_f64_u (f64.const 0x0.0000000000001p-1022)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 43))
		))

		(if (i32.ne (call $i32.trunc_f64_u (f64.const -0x0.0000000000001p-1022)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 44))
		))

		(if (i32.ne (call $i32.trunc_f64_u (f64.const 1.0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 45))
		))

		(if (i32.ne (call $i32.trunc_f64_u (f64.const 0x1.199999999999ap+0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 46))
		))

		(if (i32.ne (call $i32.trunc_f64_u (f64.const 1.5)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 47))
		))

		(if (i32.ne (call $i32.trunc_f64_u (f64.const 1.9)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 48))
		))

		(if (i32.ne (call $i32.trunc_f64_u (f64.const 2.0)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 49))
		))

		 ;; 0x1.00000p+31 -> 8000 0000
		(if (i32.ne (call $i32.trunc_f64_u (f64.const 2147483648)) (i32.const -2147483648)) (then     
			(call $proc_exit (i32.const 50))
		))

		(call $i32.trunc_f64_u (f64.const 4294967295.0))
		drop

		(if (i32.ne (call $i32.trunc_f64_u (f64.const 4294967295.0)) (i32.const -1)) (then
			(call $proc_exit (i32.const 51))
		))

		(if (i32.ne (call $i32.trunc_f64_u (f64.const -0x1.ccccccccccccdp-1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 52))
		))

		(if (i32.ne (call $i32.trunc_f64_u (f64.const -0x1.fffffffffffffp-1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 53))
		))

		(if (i32.ne (call $i32.trunc_f64_u (f64.const 1e8)) (i32.const 100000000)) (then     
			(call $proc_exit (i32.const 54))
		))

		(if (i32.ne (call $i32.trunc_f64_u (f64.const -0.9)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 55))
		))

		(if (i32.ne (call $i32.trunc_f64_u (f64.const 4294967295.9)) (i32.const 4294967295)) (then     
			(call $proc_exit (i32.const 56))
		))

		;; (assert_trap (invoke "i32.trunc_f64_u" (f64.const 4294967296.0)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f64_u" (f64.const -1.0)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f64_u" (f64.const 1e16)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f64_u" (f64.const 1e30)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f64_u" (f64.const 9223372036854775808)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f64_u" (f64.const inf)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f64_u" (f64.const -inf)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f64_u" (f64.const nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i32.trunc_f64_u" (f64.const nan:0x4000000000000)) "invalid conversion to integer")
		;; (assert_trap (invoke "i32.trunc_f64_u" (f64.const -nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i32.trunc_f64_u" (f64.const -nan:0x4000000000000)) "invalid conversion to integer")

		(call $proc_exit (i32.const 0))
	)	
)

