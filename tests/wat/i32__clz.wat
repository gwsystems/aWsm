;; i32 operations

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
    (export "memory" (memory 0))

	(func $clz (export "clz") (param $x i32) (result i32) (i32.clz (local.get $x)))

	(func $_start (export "_start")
		(if (i32.ne (call $clz (i32.const 0xffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 166))
		))

		(if (i32.ne (call $clz (i32.const 0)) (i32.const 32)) (then     
			(call $proc_exit (i32.const 167))
		))

		(if (i32.ne (call $clz (i32.const 0x00008000)) (i32.const 16)) (then     
			(call $proc_exit (i32.const 168))
		))

		(if (i32.ne (call $clz (i32.const 0xff)) (i32.const 24)) (then     
			(call $proc_exit (i32.const 169))
		))

		(if (i32.ne (call $clz (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 170))
		))

		(if (i32.ne (call $clz (i32.const 1)) (i32.const 31)) (then     
			(call $proc_exit (i32.const 171))
		))

		(if (i32.ne (call $clz (i32.const 2)) (i32.const 30)) (then     
			(call $proc_exit (i32.const 172))
		))

		(if (i32.ne (call $clz (i32.const 0x7fffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 173))
		))

	)
)

;; Type check

;; (assert_invalid (module (func (result i32) (i32.rem_s (i64.const 0) (f32.const 0)))) "type mismatch")
