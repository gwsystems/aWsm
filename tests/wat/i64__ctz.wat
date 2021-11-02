;; i64 operations

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
    (export "memory" (memory 0))

	(func $ctz (export "ctz") (param $x i64) (result i64) (i64.ctz (local.get $x)))

	(func $_start (export "_start")
		(if (i64.ne (call $ctz (i64.const -1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i64.ne (call $ctz (i64.const 0)) (i64.const 64)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i64.ne (call $ctz (i64.const 0x00008000)) (i64.const 15)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i64.ne (call $ctz (i64.const 0x00010000)) (i64.const 16)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i64.ne (call $ctz (i64.const 0x8000000000000000)) (i64.const 63)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i64.ne (call $ctz (i64.const 0x7fffffffffffffff)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))
		

		(call $proc_exit (i32.const 0))

	)
)

;; (assert_invalid (module (func (result i64) (i64.ctz (i32.const 0)))) "type mismatch")
