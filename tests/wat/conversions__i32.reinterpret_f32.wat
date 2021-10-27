(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)

	(func $i32.reinterpret_f32 (export "i32.reinterpret_f32") (param $x f32) (result i32) (i32.reinterpret_f32 (local.get $x)))

	(func $_start (export "_start")
		
		(if (i32.ne (call $i32.reinterpret_f32 (f32.const 0.0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const -0.0)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 2))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const 0x1p-149)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 3))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const -nan:0x7fffff)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 4))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const -0x1p-149)) (i32.const 0x80000001)) (then     
			(call $proc_exit (i32.const 5))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const 1.0)) (i32.const 1065353216)) (then     
			(call $proc_exit (i32.const 6))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const 3.1415926)) (i32.const 1078530010)) (then     
			(call $proc_exit (i32.const 7))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const 0x1.fffffep+127)) (i32.const 2139095039)) (then     
			(call $proc_exit (i32.const 8))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const -0x1.fffffep+127)) (i32.const -8388609)) (then     
			(call $proc_exit (i32.const 9))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const inf)) (i32.const 0x7f800000)) (then     
			(call $proc_exit (i32.const 10))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const -inf)) (i32.const 0xff800000)) (then     
			(call $proc_exit (i32.const 11))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const nan)) (i32.const 0x7fc00000)) (then     
			(call $proc_exit (i32.const 12))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const -nan)) (i32.const 0xffc00000)) (then     
			(call $proc_exit (i32.const 13))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const nan:0x200000)) (i32.const 0x7fa00000)) (then     
			(call $proc_exit (i32.const 14))
		))

		(if (i32.ne (call $i32.reinterpret_f32 (f32.const -nan:0x200000)) (i32.const 0xffa00000)) (then     
			(call $proc_exit (i32.const 15))
		))

		(call $proc_exit (i32.const 0))
	)	
)

