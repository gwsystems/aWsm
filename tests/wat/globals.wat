(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory (export "memory") 1)
	(global (mut i32) (i32.const 4))
	(global (mut i32) (i32.const 8))
	(global (mut i32) (i32.const 15))
	(global (mut i32) (i32.const 16))
	(global (mut i32) (i32.const 23))
	(global (mut i32) (i32.const 42))
	(global (mut i64) (i64.const 42))
	(global (mut f32) (f32.const 3.14))
	(global (mut f64) (f64.const 3.14))

	(func $_start (export "_start")
		(global.set 0 (i32.mul (global.get 0) (i32.const 2)))
		(global.set 1 (i32.mul (global.get 1) (i32.const 2)))
		(global.set 2 (i32.mul (global.get 2) (i32.const 2)))
		(global.set 3 (i32.mul (global.get 3) (i32.const 2)))
		(global.set 4 (i32.mul (global.get 4) (i32.const 2)))
		(global.set 5 (i32.mul (global.get 5) (i32.const 2)))
		(global.set 6 (i64.mul (global.get 6) (i64.const 2)))
		(global.set 7 (f32.mul (global.get 7) (f32.const 2)))
		(global.set 8 (f64.mul (global.get 8) (f64.const 2)))

		(if (i32.ne (global.get 0) (i32.const 8)) (then     
			(call $proc_exit (i32.const 1))
		))
		(if (i32.ne (global.get 1) (i32.const 16)) (then     
			(call $proc_exit (i32.const 1))
		))
		(if (i32.ne (global.get 2) (i32.const 30)) (then     
			(call $proc_exit (i32.const 1))
		))
		(if (i32.ne (global.get 3) (i32.const 32)) (then     
			(call $proc_exit (i32.const 1))
		))
		(if (i32.ne (global.get 4) (i32.const 46)) (then     
			(call $proc_exit (i32.const 1))
		))
		(if (i32.ne (global.get 5) (i32.const 84)) (then     
			(call $proc_exit (i32.const 1))
		))
		(if (i64.ne (global.get 6) (i64.const 84)) (then     
			(call $proc_exit (i32.const 1))
		))
		(if (f32.ne (global.get 7) (f32.const 6.28)) (then     
			(call $proc_exit (i32.const 1))
		))
		(if (f64.ne (global.get 8) (f64.const 6.28)) (then     
			(call $proc_exit (i32.const 1))
		))

		(call $proc_exit (i32.const 0))
	)
)
