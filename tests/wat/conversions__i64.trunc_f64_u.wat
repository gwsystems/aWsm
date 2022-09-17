(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
    (export "memory" (memory 0))

	(func $i64.trunc_f64_u (export "i64.trunc_f64_u") (param $x f64) (result i64) (i64.trunc_f64_u (local.get $x)))

	(func $_start (export "_start")
		
		(if (i64.ne (call $i64.trunc_f64_u (f64.const 0.0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const -0.0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const 0x0.0000000000001p-1022)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const -0x0.0000000000001p-1022)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const 1.0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const 0x1.199999999999ap+0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const 1.5)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const 4294967295)) (i64.const 0xffffffff)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const 4294967296)) (i64.const 0x100000000)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const 18446744073709549568.0)) (i64.const -2048)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const -0x1.ccccccccccccdp-1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const -0x1.fffffffffffffp-1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const 1e8)) (i64.const 100000000)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const 1e16)) (i64.const 10000000000000000)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f64_u (f64.const 9223372036854775808)) (i64.const -9223372036854775808)) (then     
			(call $proc_exit (i32.const 41))
		))

		;; (assert_trap (invoke "i64.trunc_f64_u" (f64.const 18446744073709551616.0)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f64_u" (f64.const -1.0)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f64_u" (f64.const inf)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f64_u" (f64.const -inf)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f64_u" (f64.const nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i64.trunc_f64_u" (f64.const nan:0x4000000000000)) "invalid conversion to integer")
		;; (assert_trap (invoke "i64.trunc_f64_u" (f64.const -nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i64.trunc_f64_u" (f64.const -nan:0x4000000000000)) "invalid conversion to integer")


		(call $proc_exit (i32.const 0))
	)	
)

