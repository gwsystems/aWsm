(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
    (export "memory" (memory 0))

	(func $i64.trunc_f32_u (export "i64.trunc_f32_u") (param $x f32) (result i64) (i64.trunc_f32_u (local.get $x)))

	(func $_start (export "_start")
		(if (i64.ne (call $i64.trunc_f32_u (f32.const 0.0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64.trunc_f32_u (f32.const -0.0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 42))
		))

		(if (i64.ne (call $i64.trunc_f32_u (f32.const 0x1p-149)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 43))
		))

		(if (i64.ne (call $i64.trunc_f32_u (f32.const -0x1p-149)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 44))
		))

		(if (i64.ne (call $i64.trunc_f32_u (f32.const 1.0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 45))
		))

		(if (i64.ne (call $i64.trunc_f32_u (f32.const 0x1.19999ap+0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 46))
		))

		(if (i64.ne (call $i64.trunc_f32_u (f32.const 1.5)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 47))
		))

		(if (i64.ne (call $i64.trunc_f32_u (f32.const 4294967296)) (i64.const 4294967296)) (then     
			(call $proc_exit (i32.const 48))
		))

		(if (i64.ne (call $i64.trunc_f32_u (f32.const 18446742974197923840.0)) (i64.const -1099511627776)) (then     
			(call $proc_exit (i32.const 49))
		))

		(if (i64.ne (call $i64.trunc_f32_u (f32.const -0x1.ccccccp-1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 50))
		))

		(if (i64.ne (call $i64.trunc_f32_u (f32.const -0x1.fffffep-1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 51))
		))


		;; (assert_trap (invoke "i64.trunc_f32_u" (f32.const 18446744073709551616.0)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f32_u" (f32.const -1.0)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f32_u" (f32.const inf)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f32_u" (f32.const -inf)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f32_u" (f32.const nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i64.trunc_f32_u" (f32.const nan:0x200000)) "invalid conversion to integer")
		;; (assert_trap (invoke "i64.trunc_f32_u" (f32.const -nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i64.trunc_f32_u" (f32.const -nan:0x200000)) "invalid conversion to integer")


		(call $proc_exit (i32.const 0))
	)	
)

