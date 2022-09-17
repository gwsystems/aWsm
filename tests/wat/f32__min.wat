(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
    (export "memory" (memory 0))

	(func $min (export "min") (param $x f32) (param $y f32) (result f32) (f32.min (local.get $x) (local.get $y)))
	
	(func $_start (export "_start")
		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const -0x0p+0)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const 0x0p+0)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const -0x0p+0)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const 0x0p+0)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const -0x1p-149)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const 0x1p-149)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const -0x1p-149)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const 0x1p-149)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const -0x1p-126)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const 0x1p-126)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const -0x1p-126)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const 0x1p-126)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const -0x1p-1)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const 0x1p-1)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const -0x1p-1)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const 0x1p-1)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const -0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const 0x1p+0)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const -0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const 0x1p+0)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const 0x1.921fb6p+2)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const 0x1.921fb6p+2)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const 0x1.fffffep+127)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const 0x1.fffffep+127)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x0p+0) (f32.const inf)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x0p+0) (f32.const inf)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		;; (if (f32.ne (call $min (f32.const -0x0p+0) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x0p+0) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x0p+0) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x0p+0) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x0p+0) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x0p+0) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x0p+0) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x0p+0) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const -0x0p+0)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const 0x0p+0)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const -0x0p+0)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const 0x0p+0)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const -0x1p-149)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const 0x1p-149)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const -0x1p-149)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const 0x1p-149)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const -0x1p-126)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const 0x1p-126)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const -0x1p-126)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const 0x1p-126)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const -0x1p-1)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const 0x1p-1)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const -0x1p-1)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const 0x1p-1)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const -0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const 0x1p+0)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const -0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const 0x1p+0)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const 0x1.921fb6p+2)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const 0x1.921fb6p+2)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const 0x1.fffffep+127)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const 0x1.fffffep+127)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-149) (f32.const inf)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-149) (f32.const inf)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		;; (if (f32.ne (call $min (f32.const -0x1p-149) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1p-149) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1p-149) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1p-149) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p-149) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p-149) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p-149) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p-149) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const -0x0p+0)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const 0x0p+0)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const -0x0p+0)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const 0x0p+0)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const -0x1p-149)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const 0x1p-149)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const -0x1p-149)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const 0x1p-149)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const -0x1p-126)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const 0x1p-126)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const -0x1p-126)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const 0x1p-126)) (f32.const 0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const -0x1p-1)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const 0x1p-1)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const -0x1p-1)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const 0x1p-1)) (f32.const 0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const -0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const 0x1p+0)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const -0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const 0x1p+0)) (f32.const 0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const 0x1.921fb6p+2)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const 0x1.921fb6p+2)) (f32.const 0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const 0x1.fffffep+127)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const 0x1.fffffep+127)) (f32.const 0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-126) (f32.const inf)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-126) (f32.const inf)) (f32.const 0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		;; (if (f32.ne (call $min (f32.const -0x1p-126) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1p-126) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1p-126) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1p-126) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p-126) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p-126) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p-126) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p-126) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const -0x0p+0)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const 0x0p+0)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const -0x0p+0)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const 0x0p+0)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const -0x1p-149)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const 0x1p-149)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const -0x1p-149)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const 0x1p-149)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const -0x1p-126)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const 0x1p-126)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const -0x1p-126)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const 0x1p-126)) (f32.const 0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const -0x1p-1)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const 0x1p-1)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const -0x1p-1)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const 0x1p-1)) (f32.const 0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const -0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const 0x1p+0)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const -0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const 0x1p+0)) (f32.const 0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const 0x1.921fb6p+2)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const 0x1.921fb6p+2)) (f32.const 0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const 0x1.fffffep+127)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const 0x1.fffffep+127)) (f32.const 0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p-1) (f32.const inf)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p-1) (f32.const inf)) (f32.const 0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		;; (if (f32.ne (call $min (f32.const -0x1p-1) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1p-1) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1p-1) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1p-1) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p-1) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p-1) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p-1) (f32.const nan)) (f32.const nan:canonical))(then     
			;; (call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p-1) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
			;; (call $proc_exit (i32.const 1))
		;; ))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const -0x0p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const 0x0p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const -0x0p+0)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const 0x0p+0)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const -0x1p-149)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const 0x1p-149)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const -0x1p-149)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const 0x1p-149)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const -0x1p-126)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const 0x1p-126)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const -0x1p-126)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const 0x1p-126)) (f32.const 0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const -0x1p-1)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const 0x1p-1)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const -0x1p-1)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const 0x1p-1)) (f32.const 0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const -0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const 0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const -0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const 0x1p+0)) (f32.const 0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const 0x1.921fb6p+2)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const 0x1.921fb6p+2)) (f32.const 0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const 0x1.fffffep+127)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const 0x1.fffffep+127)) (f32.const 0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1p+0) (f32.const inf)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1p+0) (f32.const inf)) (f32.const 0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		;; (if (f32.ne (call $min (f32.const -0x1p+0) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1p+0) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1p+0) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1p+0) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p+0) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p+0) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p+0) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1p+0) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const -0x0p+0)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const 0x0p+0)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const -0x0p+0)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const 0x0p+0)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const -0x1p-149)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const 0x1p-149)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const -0x1p-149)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const 0x1p-149)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const -0x1p-126)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const 0x1p-126)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const -0x1p-126)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const 0x1p-126)) (f32.const 0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const -0x1p-1)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const 0x1p-1)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const -0x1p-1)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const 0x1p-1)) (f32.const 0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const -0x1p+0)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const 0x1p+0)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const -0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const 0x1p+0)) (f32.const 0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const 0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const 0x1.921fb6p+2)) (f32.const 0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const 0x1.fffffep+127)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const 0x1.fffffep+127)) (f32.const 0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const inf)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const inf)) (f32.const 0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		;; (if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1.921fb6p+2) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1.921fb6p+2) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const -0x0p+0)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const 0x0p+0)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const -0x0p+0)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const 0x0p+0)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const -0x1p-149)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const 0x1p-149)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const -0x1p-149)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const 0x1p-149)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const -0x1p-126)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const 0x1p-126)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const -0x1p-126)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const 0x1p-126)) (f32.const 0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const -0x1p-1)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const 0x1p-1)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const -0x1p-1)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const 0x1p-1)) (f32.const 0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const -0x1p+0)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const 0x1p+0)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const -0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const 0x1p+0)) (f32.const 0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const 0x1.921fb6p+2)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const 0x1.921fb6p+2)) (f32.const 0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const 0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const 0x1.fffffep+127)) (f32.const 0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const inf)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const inf)) (f32.const 0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		;; (if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -0x1.fffffep+127) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const 0x1.fffffep+127) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		(if (f32.ne (call $min (f32.const -inf) (f32.const -0x0p+0)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const 0x0p+0)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const -0x0p+0)) (f32.const -0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const 0x0p+0)) (f32.const 0x0p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const -0x1p-149)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const 0x1p-149)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const -0x1p-149)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const 0x1p-149)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const -0x1p-126)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const 0x1p-126)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const -0x1p-126)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const 0x1p-126)) (f32.const 0x1p-126))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const -0x1p-1)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const 0x1p-1)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const -0x1p-1)) (f32.const -0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const 0x1p-1)) (f32.const 0x1p-1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const -0x1p+0)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const 0x1p+0)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const -0x1p+0)) (f32.const -0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const 0x1p+0)) (f32.const 0x1p+0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const -0x1.921fb6p+2)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const 0x1.921fb6p+2)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const -0x1.921fb6p+2)) (f32.const -0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const 0x1.921fb6p+2)) (f32.const 0x1.921fb6p+2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const -0x1.fffffep+127)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const 0x1.fffffep+127)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (f32.ne (call $min (f32.const inf) (f32.const 0x1.fffffep+127)) (f32.const 0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const -inf) (f32.const inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $min (f32.const inf) (f32.const inf)) (f32.const inf))(then     
			(call $proc_exit (i32.const 1))
		))

		;; (if (f32.ne (call $min (f32.const -inf) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -inf) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -inf) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -inf) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const inf) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const inf) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const inf) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const inf) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const -0x0p+0)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const -0x0p+0)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const 0x0p+0)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const 0x0p+0)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const -0x0p+0)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const -0x0p+0)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const 0x0p+0)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const 0x0p+0)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const -0x1p-149)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const -0x1p-149)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const 0x1p-149)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const 0x1p-149)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const -0x1p-149)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const -0x1p-149)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const 0x1p-149)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const 0x1p-149)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const -0x1p-126)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const -0x1p-126)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const 0x1p-126)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const 0x1p-126)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const -0x1p-126)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const -0x1p-126)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const 0x1p-126)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const 0x1p-126)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const -0x1p-1)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const -0x1p-1)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const 0x1p-1)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const 0x1p-1)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const -0x1p-1)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const -0x1p-1)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const 0x1p-1)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const 0x1p-1)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const -0x1p+0)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const -0x1p+0)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const 0x1p+0)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const 0x1p+0)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const -0x1p+0)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const -0x1p+0)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const 0x1p+0)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const 0x1p+0)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const -0x1.921fb6p+2)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const -0x1.921fb6p+2)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const 0x1.921fb6p+2)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const 0x1.921fb6p+2)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const -0x1.921fb6p+2)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const -0x1.921fb6p+2)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const 0x1.921fb6p+2)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const 0x1.921fb6p+2)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const -0x1.fffffep+127)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const -0x1.fffffep+127)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const 0x1.fffffep+127)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const 0x1.fffffep+127)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const -0x1.fffffep+127)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const -0x1.fffffep+127)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const 0x1.fffffep+127)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const 0x1.fffffep+127)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const -inf)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const -inf)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const inf)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const inf)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const -inf)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const -inf)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const inf)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const inf)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const -nan)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const nan)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const -nan:0x200000) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const -nan)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const -nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const nan)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		;; (if (f32.ne (call $min (f32.const nan:0x200000) (f32.const nan:0x200000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

		(call $proc_exit (i32.const 0))

	)
)
