(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
	(global i32 (i32.const 4))
	(global i32 (i32.const 8))
	(global i32 (i32.const 15))
	(global i32 (i32.const 16))
	(global i32 (i32.const 23))
	(global i32 (i32.const 42))

	(func $_start (export "_start")
		(global.set (i32.const 0) (i32.mul (global.get 0) (i32.const 2)))
		(global.set (i32.const 1) (i32.mul (global.get 1) (i32.const 2)))
		(global.set (i32.const 2) (i32.mul (global.get 2) (i32.const 2)))
		(global.set (i32.const 3) (i32.mul (global.get 3) (i32.const 2)))
		(global.set (i32.const 4) (i32.mul (global.get 4) (i32.const 2)))
		(global.set (i32.const 5) (i32.mul (global.get 5) (i32.const 2)))

		(if (i32.ne (global.get (i32.const 0)) (i32.const 8)) (then     
			(call $proc_exit (i32.const 1))
		))
		(if (i32.ne (global.get (i32.const 1)) (i32.const 16)) (then     
			(call $proc_exit (i32.const 1))
		))
		(if (i32.ne (global.get (i32.const 2)) (i32.const 30)) (then     
			(call $proc_exit (i32.const 1))
		))
		(if (i32.ne (global.get (i32.const 3)) (i32.const 32)) (then     
			(call $proc_exit (i32.const 1))
		))
		(if (i32.ne (global.get (i32.const 4)) (i32.const 46)) (then     
			(call $proc_exit (i32.const 1))
		))
		(if (i32.ne (global.get (i32.const 5)) (i32.const 84)) (then     
			(call $proc_exit (i32.const 1))
		))

		(call $proc_exit (i32.const 0))
	)
)
