(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)

	(func $i32.trunc_f32_u (export "i32.trunc_f32_u") (param $x f32) (result i32) (i32.trunc_f32_u (local.get $x)))

	(func $_start (export "_start")
		(if (i32.ne (call $i32.trunc_f32_u (f32.const 0.0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 26))
		))

		(if (i32.ne (call $i32.trunc_f32_u (f32.const -0.0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 27))
		))
		
		(if (i32.ne (call $i32.trunc_f32_u (f32.const 0x1p-149)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 28))
		))
		
		(if (i32.ne (call $i32.trunc_f32_u (f32.const -0x1p-149)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 29))
		))
		
		(if (i32.ne (call $i32.trunc_f32_u (f32.const 1.0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 30))
		))
		
		(if (i32.ne (call $i32.trunc_f32_u (f32.const 0x1.19999ap+0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 31))
		))
		
		(if (i32.ne (call $i32.trunc_f32_u (f32.const 1.5)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 32))
		))
		
		(if (i32.ne (call $i32.trunc_f32_u (f32.const 1.9)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 33))
		))
		
		(if (i32.ne (call $i32.trunc_f32_u (f32.const 2.0)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 34))
		))
		
		;; 0x1.00000p+31 -> 8000 0000
		(if (i32.ne (call $i32.trunc_f32_u (f32.const 2147483648)) (i32.const -2147483648))  (then     
			(call $proc_exit (i32.const 35))
		))
		
		(if (i32.ne (call $i32.trunc_f32_u (f32.const 4294967040.0)) (i32.const -256)) (then     
			(call $proc_exit (i32.const 36))
		))
		
		(if (i32.ne (call $i32.trunc_f32_u (f32.const -0x1.ccccccp-1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 37))
		))
		
		(if (i32.ne (call $i32.trunc_f32_u (f32.const -0x1.fffffep-1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 38))
		))

		;; (assert_trap (invoke "i32.trunc_f32_u" (f32.const 4294967296.0)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f32_u" (f32.const -1.0)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f32_u" (f32.const inf)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f32_u" (f32.const -inf)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f32_u" (f32.const nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i32.trunc_f32_u" (f32.const nan:0x200000)) "invalid conversion to integer")
		;; (assert_trap (invoke "i32.trunc_f32_u" (f32.const -nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i32.trunc_f32_u" (f32.const -nan:0x200000)) "invalid conversion to integer")

		(call $proc_exit (i32.const 0))
	)	
)

