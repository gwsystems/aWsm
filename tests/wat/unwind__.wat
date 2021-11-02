;; Test that control-flow transfer unwinds stack and it can be anything after.

(module
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)

	(func $func-unwind-by-unreachable (export "func-unwind-by-unreachable")
		(i32.const 3) (i64.const 1) (unreachable)
	)

	(func $func-unwind-by-br (export "func-unwind-by-br")
		(i32.const 3) (i64.const 1) (br 0)
	)

	(func $func-unwind-by-br-value (export "func-unwind-by-br-value") (result i32)
		(i32.const 3) (i64.const 1) (br 0 (i32.const 9))
	)
	
	(func $func-unwind-by-br_if (export "func-unwind-by-br_if")
		(i32.const 3) (i64.const 1) (drop (drop (br_if 0 (i32.const 1))))
	)
	
	(func $func-unwind-by-br_table (export "func-unwind-by-br_table")
		(i32.const 3) (i64.const 1) (br_table 0 (i32.const 0))
	)
	
	(func $func-unwind-by-br_table-value (export "func-unwind-by-br_table-value") (result i32)
		(i32.const 3) (i64.const 1) (br_table 0 (i32.const 9) (i32.const 0))
	)
	
	(func $func-unwind-by-return (export "func-unwind-by-return") (result i32)
		(i32.const 3) (i64.const 1) (return (i32.const 9))
	)

	(func $block-unwind-by-unreachable (export "block-unwind-by-unreachable")
		(block (i32.const 3) (i64.const 1) (unreachable))
	)
	
	(func $block-unwind-by-br (export "block-unwind-by-br") (result i32)
		(block (i32.const 3) (i64.const 1) (br 0)) (i32.const 9)
	)
	
	(func $block-unwind-by-br-value (export "block-unwind-by-br-value") (result i32)
		(block (result i32) (i32.const 3) (i64.const 1) (br 0 (i32.const 9)))
	)
	
	(func $block-unwind-by-br_if (export "block-unwind-by-br_if") (result i32)
		(block (i32.const 3) (i64.const 1) (drop (drop (br_if 0 (i32.const 1))))) (i32.const 9)
	)
	
	(func $block-unwind-by-br_if-value (export "block-unwind-by-br_if-value") (result i32)
		(block (result i32)
			(i32.const 3) (i64.const 1) (drop (drop (br_if 0 (i32.const 9) (i32.const 1))))
		)
	)
	
	(func $block-unwind-by-br_table (export "block-unwind-by-br_table") (result i32)
		(block (i32.const 3) (i64.const 1) (br_table 0 (i32.const 0))) (i32.const 9)
	)
	
	(func $block-unwind-by-br_table-value (export "block-unwind-by-br_table-value") (result i32)
		(block (result i32)
			(i32.const 3) (i64.const 1) (br_table 0 (i32.const 9) (i32.const 0))
		)
	)
	
	(func $block-unwind-by-return (export "block-unwind-by-return") (result i32)
		(block (result i32) (i32.const 3) (i64.const 1) (return (i32.const 9)))
	)

	(func $block-nested-unwind-by-unreachable (export "block-nested-unwind-by-unreachable") (result i32)
		(block (result i32) (i32.const 3) (block (i64.const 1) (unreachable)))
	)
	
	(func $block-nested-unwind-by-br (export "block-nested-unwind-by-br") (result i32)
		(block (i32.const 3) (block (i64.const 1) (br 1)) (drop)) (i32.const 9)
	)
	
	(func $block-nested-unwind-by-br-value (export "block-nested-unwind-by-br-value") (result i32)
		(block (result i32)
			(i32.const 3) (block (i64.const 1) (br 1 (i32.const 9)))
		)
	)
	
	(func $block-nested-unwind-by-br_if (export "block-nested-unwind-by-br_if") (result i32)
		(block (i32.const 3) (block (i64.const 1) (drop (br_if 1 (i32.const 1)))) (drop)) (i32.const 9)
	)
	
	(func $block-nested-unwind-by-br_if-value (export "block-nested-unwind-by-br_if-value") (result i32)
		(block (result i32)
			(i32.const 3) (block (i64.const 1) (drop (drop (br_if 1 (i32.const 9) (i32.const 1)))))
		)
	)
	
	(func $block-nested-unwind-by-br_table (export "block-nested-unwind-by-br_table") (result i32)
		(block
			(i32.const 3) (block (i64.const 1) (br_table 1 (i32.const 1)))
			(drop)
		)
		(i32.const 9)
	)

	(func $block-nested-unwind-by-br_table-value (export "block-nested-unwind-by-br_table-value") (result i32)
		(block (result i32)
			(i32.const 3)
			(block (i64.const 1) (br_table 1 (i32.const 9) (i32.const 1)))
		)
	)
	
	(func $block-nested-unwind-by-return (export "block-nested-unwind-by-return") (result i32)
		(block (result i32)
			(i32.const 3) (block (i64.const 1) (return (i32.const 9)))
		)
	)

	(func $unary-after-unreachable (export "unary-after-unreachable") (result i32)
		(f32.const 0) (unreachable) (i64.eqz)
	)
	
	(func $unary-after-br (export "unary-after-br") (result i32)
		(block (result i32) (f32.const 0) (br 0 (i32.const 9)) (i64.eqz))
	)
	
	(func $unary-after-br_if (export "unary-after-br_if") (result i32)
		(block (result i32)
			(i64.const 0) (drop (br_if 0 (i32.const 9) (i32.const 1))) (i64.eqz)
		)
	)
	
	(func $unary-after-br_table (export "unary-after-br_table") (result i32)
		(block (result i32)
			(f32.const 0) (br_table 0 0 (i32.const 9) (i32.const 0)) (i64.eqz)
		)
	)
	
	(func $unary-after-return (export "unary-after-return") (result i32)
		(f32.const 0) (return (i32.const 9)) (i64.eqz)
	)

	(func $binary-after-unreachable (export "binary-after-unreachable") (result i32)
		(f32.const 0) (f64.const 1) (unreachable) (i64.eq)
	)
	
	(func $binary-after-br (export "binary-after-br") (result i32)
		(block (result i32)
			(f32.const 0) (f64.const 1) (br 0 (i32.const 9)) (i64.eq)
		)
	)
	
	(func $binary-after-br_if (export "binary-after-br_if") (result i32)
		(block (result i32)
			(i64.const 0) (i64.const 1) (drop (br_if 0 (i32.const 9) (i32.const 1)))
			(i64.eq)
		)
	)

	(func $binary-after-br_table (export "binary-after-br_table") (result i32)
		(block (result i32)
			(f32.const 0) (f64.const 1) (br_table 0 (i32.const 9) (i32.const 0))
			(i64.eq)
		)
	)
	
	(func $binary-after-return (export "binary-after-return") (result i32)
		(f32.const 0) (f64.const 1) (return (i32.const 9)) (i64.eq)
	)

	(func $select-after-unreachable (export "select-after-unreachable") (result i32)
		(f32.const 0) (f64.const 1) (i64.const 0) (unreachable) (select)
	)

	(func $select-after-br (export "select-after-br") (result i32)
		(block (result i32)
			(f32.const 0) (f64.const 1) (i64.const 0) (br 0 (i32.const 9)) (select)
		)
	)

	(func $select-after-br_if (export "select-after-br_if") (result i32)
		(block (result i32)
			(i32.const 0) (i32.const 1) (i32.const 0)
			(drop (br_if 0 (i32.const 9) (i32.const 1)))
			(select)
		)
	)
	
	(func $select-after-br_table (export "select-after-br_table") (result i32)
		(block (result i32)
			(f32.const 0) (f64.const 1) (i64.const 0)
			(br_table 0 (i32.const 9) (i32.const 0))
			(select)
		)
	)

	(func $select-after-return (export "select-after-return") (result i32)
		(f32.const 0) (f64.const 1) (i64.const 1) (return (i32.const 9)) (select)
	)

	(func $block-value-after-unreachable (export "block-value-after-unreachable") (result i32)
		(block (result i32) (f32.const 0) (unreachable))
	)
	
	(func $block-value-after-br (export "block-value-after-br") (result i32)
		(block (result i32) (f32.const 0) (br 0 (i32.const 9)))
	)
	
	(func $block-value-after-br_if (export "block-value-after-br_if") (result i32)
		(block (result i32)
			(i32.const 0) (drop (br_if 0 (i32.const 9) (i32.const 1)))
		)
	)

	(func $block-value-after-br_table (export "block-value-after-br_table") (result i32)
		(block (result i32)
			(f32.const 0) (br_table 0 0 (i32.const 9) (i32.const 0))
		)
	)

	(func $block-value-after-return (export "block-value-after-return") (result i32)
		(block (result i32) (f32.const 0) (return (i32.const 9)))
	)

	(func $loop-value-after-unreachable (export "loop-value-after-unreachable") (result i32)
		(loop (result i32) (f32.const 0) (unreachable))
	)
	
	(func $loop-value-after-br (export "loop-value-after-br") (result i32)
		(block (result i32) (loop (result i32) (f32.const 0) (br 1 (i32.const 9))))
	)

	(func $loop-value-after-br_if (export "loop-value-after-br_if") (result i32)
		(block (result i32)
			(loop (result i32)
				(i32.const 0) (drop (br_if 1 (i32.const 9) (i32.const 1)))
			)
		)
	)

	(func $loop-value-after-br_table (export "loop-value-after-br_table") (result i32)
		(block (result i32)
			(loop (result i32)
				(f32.const 0) (br_table 1 1 (i32.const 9) (i32.const 0))
			)
		)
	)

	(func $loop-value-after-return (export "loop-value-after-return") (result i32)
		(loop (result i32) (f32.const 0) (return (i32.const 9)))
	)

	;; (assert_trap (invoke "func-unwind-by-unreachable") "unreachable")
  	
	(func $_start (export "_start")

		(call $func-unwind-by-br)
		
		(if (i32.ne (call $func-unwind-by-br-value) (i32.const 9)) (then     
			(call $proc_exit (i32.const 1))
		))

		(call $func-unwind-by-br_if)


		(call $func-unwind-by-br_table)

		(if (i32.ne (call $func-unwind-by-br_table-value) (i32.const 9)) (then     
			(call $proc_exit (i32.const 2))
		))

		(if (i32.ne (call $func-unwind-by-return) (i32.const 9)) (then     
			(call $proc_exit (i32.const 3))
		))

		;; (assert_trap (invoke "block-unwind-by-unreachable") "unreachable")

		(if (i32.ne (call $block-unwind-by-br) (i32.const 9)) (then     
			(call $proc_exit (i32.const 4))
		))
		
		(if (i32.ne (call $block-unwind-by-br-value) (i32.const 9)) (then     
			(call $proc_exit (i32.const 5))
		))
		
		(if (i32.ne (call $block-unwind-by-br_if) (i32.const 9)) (then     
			(call $proc_exit (i32.const 6))
		))
		
		(if (i32.ne (call $block-unwind-by-br_if-value) (i32.const 9)) (then     
			(call $proc_exit (i32.const 7))
		))
		
		(if (i32.ne (call $block-unwind-by-br_table) (i32.const 9)) (then     
			(call $proc_exit (i32.const 8))
		))
		
		(if (i32.ne (call $block-unwind-by-br_table-value) (i32.const 9)) (then     
			(call $proc_exit (i32.const 9))
		))

		(if (i32.ne (call $block-unwind-by-return) (i32.const 9)) (then     
			(call $proc_exit (i32.const 10))
		))

		;; (assert_trap (invoke "block-nested-unwind-by-unreachable") "unreachable")

		(if (i32.ne (call $block-nested-unwind-by-br) (i32.const 9)) (then     
			(call $proc_exit (i32.const 11))
		))

		(if (i32.ne (call $block-nested-unwind-by-br-value) (i32.const 9)) (then     
			(call $proc_exit (i32.const 12))
		))

		(if (i32.ne (call $block-nested-unwind-by-br_if) (i32.const 9)) (then     
			(call $proc_exit (i32.const 13))
		))

		(if (i32.ne (call $block-nested-unwind-by-br_if-value) (i32.const 9)) (then     
			(call $proc_exit (i32.const 14))
		))

		(if (i32.ne (call $block-nested-unwind-by-br_table) (i32.const 9)) (then     
			(call $proc_exit (i32.const 15))
		))

		(if (i32.ne (call $block-nested-unwind-by-br_table-value) (i32.const 9)) (then     
			(call $proc_exit (i32.const 16))
		))

		(if (i32.ne (call $block-nested-unwind-by-return) (i32.const 9)) (then     
			(call $proc_exit (i32.const 17))
		))

		;; (assert_trap (invoke "unary-after-unreachable") "unreachable")

		(if (i32.ne (call $unary-after-br) (i32.const 9)) (then     
			(call $proc_exit (i32.const 18))
		))

		(if (i32.ne (call $unary-after-br_if) (i32.const 9)) (then     
			(call $proc_exit (i32.const 19))
		))

		(if (i32.ne (call $unary-after-br_table) (i32.const 9)) (then     
			(call $proc_exit (i32.const 20))
		))

		(if (i32.ne (call $unary-after-return) (i32.const 9)) (then     
			(call $proc_exit (i32.const 21))
		))


		;; (assert_trap (invoke "binary-after-unreachable") "unreachable")

		(if (i32.ne (call $binary-after-br) (i32.const 9)) (then     
			(call $proc_exit (i32.const 22))
		))

		(if (i32.ne (call $binary-after-br_if) (i32.const 9)) (then     
			(call $proc_exit (i32.const 23))
		))

		(if (i32.ne (call $binary-after-br_table) (i32.const 9)) (then     
			(call $proc_exit (i32.const 24))
		))

		(if (i32.ne (call $binary-after-return) (i32.const 9)) (then     
			(call $proc_exit (i32.const 25))
		))


		;; (assert_trap (invoke "select-after-unreachable") "unreachable")

		(if (i32.ne (call $select-after-br) (i32.const 9)) (then     
			(call $proc_exit (i32.const 26))
		))

		(if (i32.ne (call $select-after-br_if) (i32.const 9)) (then     
			(call $proc_exit (i32.const 27))
		))

		(if (i32.ne (call $select-after-br_table) (i32.const 9)) (then     
			(call $proc_exit (i32.const 28))
		))

		(if (i32.ne (call $select-after-return) (i32.const 9)) (then     
			(call $proc_exit (i32.const 29))
		))


		;; (assert_trap (invoke "block-value-after-unreachable") "unreachable")

		(if (i32.ne (call $block-value-after-br) (i32.const 9)) (then     
			(call $proc_exit (i32.const 30))
		))

		(if (i32.ne (call $block-value-after-br_if) (i32.const 9)) (then     
			(call $proc_exit (i32.const 31))
		))

		(if (i32.ne (call $block-value-after-br_table) (i32.const 9)) (then     
			(call $proc_exit (i32.const 32))
		))

		(if (i32.ne (call $block-value-after-return) (i32.const 9)) (then     
			(call $proc_exit (i32.const 33))
		))


		;; (assert_trap (invoke "loop-value-after-unreachable") "unreachable")

		(if (i32.ne (call $loop-value-after-br) (i32.const 9)) (then     
			(call $proc_exit (i32.const 34))
		))

		(if (i32.ne (call $loop-value-after-br_if) (i32.const 9)) (then     
			(call $proc_exit (i32.const 35))
		))

		(if (i32.ne (call $loop-value-after-br_table) (i32.const 9)) (then     
			(call $proc_exit (i32.const 36))
		))

		(if (i32.ne (call $loop-value-after-return) (i32.const 9)) (then     
			(call $proc_exit (i32.const 37))
		))

		(call $proc_exit (i32.const 0))
	)
)
