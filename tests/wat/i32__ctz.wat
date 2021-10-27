;; i32 operations

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
    (export "memory" (memory 0))

	(func $ctz (export "ctz") (param $x i32) (result i32) (i32.ctz (local.get $x)))

	(func $_start (export "_start")
		(if (i32.ne (call $ctz (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 174))
		))

		(if (i32.ne (call $ctz (i32.const 0)) (i32.const 32)) (then     
			(call $proc_exit (i32.const 175))
		))

		(if (i32.ne (call $ctz (i32.const 0x00008000)) (i32.const 15)) (then     
			(call $proc_exit (i32.const 176))
		))

		(if (i32.ne (call $ctz (i32.const 0x00010000)) (i32.const 16)) (then     
			(call $proc_exit (i32.const 177))
		))

		(if (i32.ne (call $ctz (i32.const 0x80000000)) (i32.const 31)) (then     
			(call $proc_exit (i32.const 178))
		))

		(if (i32.ne (call $ctz (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 179))
		))

	)
)
