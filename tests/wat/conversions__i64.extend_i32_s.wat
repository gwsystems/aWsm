(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)

	(func $i64.extend_i32_s (export "i64.extend_i32_s") (param $x i32) (result i64) (i64.extend_i32_s (local.get $x)))

	(func $_start (export "_start")
		(if (i64.ne (call $i64.extend_i32_s (i32.const 0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $i64.extend_i32_s (i32.const 10000)) (i64.const 10000)) (then     
			(call $proc_exit (i32.const 2))
		))

		(if (i64.ne (call $i64.extend_i32_s (i32.const -1)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 3))
		))

		(if (i64.ne (call $i64.extend_i32_s (i32.const 0x7fffffff)) (i64.const 0x000000007fffffff)) (then     
			(call $proc_exit (i32.const 4))
		))

		(if (i64.ne (call $i64.extend_i32_s (i32.const 0x80000000)) (i64.const 0xffffffff80000000)) (then     
			(call $proc_exit (i32.const 5))
		))

		(call $proc_exit (i32.const 0))
	)	
)
