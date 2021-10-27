;; Test that control-flow transfer unwinds stack and it can be anything after.

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
	
	(func $func-unwind-by-br_if-value (export "func-unwind-by-br_if-value") (result i32)
		(i32.const 3) (i64.const 1) (drop (drop (br_if 0 (i32.const 9) (i32.const 1))))
	)
  	
	(func $_start (export "_start")

		(if (i32.ne (call $func-unwind-by-br_if-value) (i32.const 9)) (then     
			(call $proc_exit (i32.const 1))
		))

	)
)
