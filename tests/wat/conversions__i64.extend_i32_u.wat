(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
    (export "memory" (memory 0))

	(func $i64.extend_i32_u (export "i64.extend_i32_u") (param $x i32) (result i64) (i64.extend_i32_u (local.get $x)))

	(func $_start (export "_start")
		(if (i64.ne (call $i64.extend_i32_u (i32.const 0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $i64.extend_i32_u (i32.const 10000)) (i64.const 10000)) (then     
			(call $proc_exit (i32.const 2))
		))

		(if (i64.ne (call $i64.extend_i32_u (i32.const -10000)) (i64.const 0x00000000ffffd8f0)) (then     
			(call $proc_exit (i32.const 3))
		))

		(if (i64.ne (call $i64.extend_i32_u (i32.const -1)) (i64.const 0xffffffff)) (then     
			(call $proc_exit (i32.const 4))
		))

		(if (i64.ne (call $i64.extend_i32_u (i32.const 0x7fffffff)) (i64.const 0x000000007fffffff)) (then     
			(call $proc_exit (i32.const 5))
		))

		(if (i64.ne (call $i64.extend_i32_u (i32.const 0x80000000)) (i64.const 0x0000000080000000)) (then     
			(call $proc_exit (i32.const 5))
		))

		(call $proc_exit (i32.const 0))
	)	
)
