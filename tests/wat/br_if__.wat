;; Derived from https://github.com/WebAssembly/testsuite/blob/main/br_if.wast

(module $br_if-unit-tests
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))

	(memory 1)
	(export "memory" (memory 0))
	;; (table $tbl 0 anyfunc)

	(func $dummy)

	(func $type-i32 (export "type-i32")
		(block (drop (i32.ctz (br_if 0 (i32.const 0) (i32.const 1)))))
	)

	(func $type-i64 (export "type-i64")
		(block (drop (i64.ctz (br_if 0 (i64.const 0) (i32.const 1)))))
	)

	(func $type-f32 (export "type-f32")
		(block (drop (f32.neg (br_if 0 (f32.const 0) (i32.const 1)))))
	)
	
	(func $type-f64 (export "type-f64")
		(block (drop (f64.neg (br_if 0 (f64.const 0) (i32.const 1)))))
	)

	(func $type-i32-value (export "type-i32-value") (result i32)
		(block (result i32) (i32.ctz (br_if 0 (i32.const 1) (i32.const 1))))
	)

	(func $type-i64-value (export "type-i64-value") (result i64)
		(block (result i64) (i64.ctz (br_if 0 (i64.const 2) (i32.const 1))))
	)
	
	(func $type-f32-value (export "type-f32-value") (result f32)
		(block (result f32) (f32.neg (br_if 0 (f32.const 3) (i32.const 1))))
	)
	
	(func $type-f64-value (export "type-f64-value") (result f64)
		(block (result f64) (f64.neg (br_if 0 (f64.const 4) (i32.const 1))))
	)

	(func $as-block-first (export "as-block-first") (param i32) (result i32)
		(block (br_if 0 (local.get 0)) (return (i32.const 2))) (i32.const 3)
	)

	(func $as-block-mid (export "as-block-mid") (param i32) (result i32)
		(block (call $dummy) (br_if 0 (local.get 0)) (return (i32.const 2)))
		(i32.const 3)
	)

	(func $as-block-last (export "as-block-last") (param i32)
		(block (call $dummy) (call $dummy) (br_if 0 (local.get 0)))
	)

	(func $as-block-first-value (export "as-block-first-value") (param i32) (result i32)
		(block (result i32)
				(drop (br_if 0 (i32.const 10) (local.get 0))) (return (i32.const 11))
		)
	)

	(func $as-block-mid-value (export "as-block-mid-value") (param i32) (result i32)
		(block (result i32)
			(call $dummy)
			(drop (br_if 0 (i32.const 20) (local.get 0)))
			(return (i32.const 21))
		)
	)

	(func $as-block-last-value (export "as-block-last-value") (param i32) (result i32)
		(block (result i32)
			(call $dummy) 
			(call $dummy) 
			(br_if 0 (i32.const 11) (local.get 0))
		)
	)

	(func $as-loop-first (export "as-loop-first") (param i32) (result i32)
		(block (loop (br_if 1 (local.get 0)) (return (i32.const 2)))) (i32.const 3)
	)

	(func $as-loop-mid (export "as-loop-mid") (param i32) (result i32)
		(block (loop (call $dummy) (br_if 1 (local.get 0)) (return (i32.const 2))))
		(i32.const 4)
	)

	(func $as-loop-last (export "as-loop-last") (param i32)
		(loop (call $dummy) (br_if 1 (local.get 0)))
	)

	(func $as-br-value (export "as-br-value") (result i32)
		(block (result i32) (br 0 (br_if 0 (i32.const 1) (i32.const 2))))
	)

	(func $as-br_if-cond (export "as-br_if-cond")
		(block (br_if 0 (br_if 0 (i32.const 1) (i32.const 1))))
	)

	(func $as-br_if-value (export "as-br_if-value") (result i32)
		(block (result i32)
			(drop (br_if 0 (br_if 0 (i32.const 1) (i32.const 2)) (i32.const 3)))
			(i32.const 4)
		)
	)

	(func $as-br_if-value-cond (export "as-br_if-value-cond") (param i32) (result i32)
		(block (result i32)
			(drop (br_if 0 (i32.const 2) (br_if 0 (i32.const 1) (local.get 0))))
			(i32.const 4)
		)
	)

	(func $as-br_table-index (export "as-br_table-index")
			(block (br_table 0 0 0 (br_if 0 (i32.const 1) (i32.const 2))))
	)

	(func $as-br_table-value (export "as-br_table-value") (result i32)
		(block (result i32)
			(br_table 0 0 0 (br_if 0 (i32.const 1) (i32.const 2)) (i32.const 3)) (i32.const 4)
		)
	)

	(func $as-br_table-value-index (export "as-br_table-value-index") (result i32)
		(block (result i32)
			(br_table 0 0 (i32.const 2) (br_if 0 (i32.const 1) (i32.const 3))) 
			(i32.const 4)
		)
	)
	
	(func $as-return-value (export "as-return-value") (result i64)
		(block (result i64) (return (br_if 0 (i64.const 1) (i32.const 2))))
	)

	(func $as-if-cond (export "as-if-cond") (param i32) (result i32)
		(block (result i32)
			(if (result i32) (br_if 0 (i32.const 1) (local.get 0)) (then 
					(i32.const 2)
			) (else 
					(i32.const 3)
			))
		)
	)
	(func $as-if-then (export "as-if-then") (param i32 i32)
		(block
			(if (local.get 0) (then 
					(br_if 1 (local.get 1))
			) (else 
					(call $dummy)
			))
		)
	)

	(func $as-if-else (export "as-if-else") (param i32 i32)
		(block
			(if (local.get 0) (then 
				(call $dummy)
			) (else 
				(br_if 1 (local.get 1))
			))
		)
	)

	(func $as-select-first (export "as-select-first") (param i32) (result i32)
		(block (result i32)
			(select (br_if 0 (i32.const 3) (i32.const 10)) (i32.const 2) (local.get 0))
		)
	)

	(func $as-select-second (export "as-select-second") (param i32) (result i32)
		(block (result i32)
			(select (i32.const 1) (br_if 0 (i32.const 3) (i32.const 10)) (local.get 0))
		)
	)
	(func $as-select-cond (export "as-select-cond") (result i32)
		(block (result i32)
			(select (i32.const 1) (i32.const 2) (br_if 0 (i32.const 3) (i32.const 10)))
		)
	)

	(func $f (param i32 i32 i32) (result i32) (i32.const -1))

	(func $as-call-first (export "as-call-first") (result i32)
		(block (result i32)
			(call $f
				(br_if 0 (i32.const 12) (i32.const 1)) (i32.const 2) (i32.const 3)
			)
		)
	)
	
	(func $as-call-mid (export "as-call-mid") (result i32)
		(block (result i32)
			(call $f
				(i32.const 1) (br_if 0 (i32.const 13) (i32.const 1)) (i32.const 3)
			)
		)
	)

	(func $as-call-last (export "as-call-last") (result i32)
		(block (result i32)
			(call $f
				(i32.const 1) (i32.const 2) (br_if 0 (i32.const 14) (i32.const 1))
			)
		)
	)

	(func $func (param i32 i32 i32) (result i32) (local.get 0))
	
	(type $check (func (param i32 i32 i32) (result i32)))
	
	(table funcref (elem $func))

	(func $as-call_indirect-func (export "as-call_indirect-func") (result i32)
		(block (result i32)
			(call_indirect (type $check)
				(br_if 0 (i32.const 4) (i32.const 10))
				(i32.const 1) (i32.const 2) (i32.const 0)
			)
		)
	)

	(func $as-call_indirect-first (export "as-call_indirect-first") (result i32)
		(block (result i32)
			(call_indirect (type $check)
				(i32.const 1) (br_if 0 (i32.const 4) (i32.const 10)) (i32.const 2) (i32.const 0)
			)
		)
	)

	(func $as-call_indirect-mid (export "as-call_indirect-mid") (result i32)
		(block (result i32)
			(call_indirect (type $check)
				(i32.const 1) (i32.const 2) (br_if 0 (i32.const 4) (i32.const 10)) (i32.const 0)
			)
		)
	)

	(func $as-call_indirect-last (export "as-call_indirect-last") (result i32)
		(block (result i32)
			(call_indirect (type $check)
				(i32.const 1) (i32.const 2) (i32.const 3) (br_if 0 (i32.const 4) (i32.const 10))
			)
		)
	)

	(func $as-local.set-value (export "as-local.set-value") (param i32) (result i32)
		(local i32)
		(block (result i32)
			(local.set 0 (br_if 0 (i32.const 17) (local.get 0)))
			(i32.const -1)
		)
	)

	(func $as-local.tee-value (export "as-local.tee-value") (param i32) (result i32)
		(block (result i32)
			(local.tee 0 (br_if 0 (i32.const 1) (local.get 0)))
			(return (i32.const -1))
		)
	)
	(global $a (mut i32) (i32.const 10))
	(func $as-global.set-value (export "as-global.set-value") (param i32) (result i32)
		(block (result i32)
			(global.set $a (br_if 0 (i32.const 1) (local.get 0)))
			(return (i32.const -1))
		)
	)

	(func $as-load-address (export "as-load-address") (result i32)
		(block (result i32) (i32.load (br_if 0 (i32.const 1) (i32.const 1))))
	)
	(func $as-loadN-address (export "as-loadN-address") (result i32)
		(block (result i32) (i32.load8_s (br_if 0 (i32.const 30) (i32.const 1))))
	)

	(func $as-store-address (export "as-store-address") (result i32)
		(block (result i32)
			(i32.store (br_if 0 (i32.const 30) (i32.const 1)) (i32.const 7)) (i32.const -1)
		)
	)
	(func $as-store-value (export "as-store-value") (result i32)
		(block (result i32)
			(i32.store (i32.const 2) (br_if 0 (i32.const 31) (i32.const 1))) (i32.const -1)
		)
	)

	(func $as-storeN-address (export "as-storeN-address") (result i32)
		(block (result i32)
			(i32.store8 (br_if 0 (i32.const 32) (i32.const 1)) (i32.const 7)) (i32.const -1)
		)
	)
	(func $as-storeN-value (result i32)
		(block (result i32)
			(i32.store16 (i32.const 2) (br_if 0 (i32.const 33) (i32.const 1))) (i32.const -1)
		)
	)

	(func $as-unary-operand (export "as-unary-operand") (result f64)
		(block (result f64) (f64.neg (br_if 0 (f64.const 1.0) (i32.const 1))))
	)
	(func $as-binary-left (export "as-binary-left") (result i32)
		(block (result i32) (i32.add (br_if 0 (i32.const 1) (i32.const 1)) (i32.const 10)))
	)
	(func $as-binary-right (export "as-binary-right") (result i32)
		(block (result i32) (i32.sub (i32.const 10) (br_if 0 (i32.const 1) (i32.const 1))))
	)
	(func $as-test-operand (export "as-test-operand") (result i32)
		(block (result i32) (i32.eqz (br_if 0 (i32.const 0) (i32.const 1))))
	)
	(func $as-compare-left (export "as-compare-left") (result i32)
		(block (result i32) (i32.le_u (br_if 0 (i32.const 1) (i32.const 1)) (i32.const 10)))
	)
	(func $as-compare-right (export "as-compare-right") (result i32)
		(block (result i32) (i32.ne (i32.const 10) (br_if 0 (i32.const 1) (i32.const 42))))
	)

	(func $as-memory.grow-size (export "as-memory.grow-size") (result i32)
		(block (result i32) (memory.grow (br_if 0 (i32.const 1) (i32.const 1))))
	)

	(func $nested-block-value (export "nested-block-value") (param i32) (result i32)
		(i32.add
			(i32.const 1)
			(block (result i32)
				(drop (i32.const 2))
				(i32.add
					(i32.const 4)
					(block (result i32)
						(drop (br_if 1 (i32.const 8) (local.get 0)))
						(i32.const 16)
					)
				)
			)
		)
	)

	(func $nested-br-value (export "nested-br-value") (param i32) (result i32)
		(i32.add
			(i32.const 1)
			(block (result i32)
				(drop (i32.const 2))
				(br 0
					(block (result i32)
						(drop (br_if 1 (i32.const 8) (local.get 0))) (i32.const 4)
					)
				)
				(i32.const 16)
			)
		)
	)

	(func $nested-br_if-value (export "nested-br_if-value") (param i32) (result i32)
		(i32.add
			(i32.const 1)
			(block (result i32)
				(drop (i32.const 2))
				(drop (br_if 0
					(block (result i32)
						(drop (br_if 1 (i32.const 8) (local.get 0))) (i32.const 4)
					)
					(i32.const 1)
				))
				(i32.const 16)
			)
		)
	)

	(func $nested-br_if-value-cond (export "nested-br_if-value-cond") (param i32) (result i32)
		(i32.add
			(i32.const 1)
			(block (result i32)
				(drop (i32.const 2))
				(drop (br_if 0
					(i32.const 4)
					(block (result i32)
						(drop (br_if 1 (i32.const 8) (local.get 0))) (i32.const 1)
					)
				))
				(i32.const 16)
			)
		)
	)

	(func $nested-br_table-value (export "nested-br_table-value") (param i32) (result i32)
		(i32.add
			(i32.const 1)
			(block (result i32)
				(drop (i32.const 2))
				(br_table 0
					(block (result i32)
						(drop (br_if 1 (i32.const 8) (local.get 0))) (i32.const 4)
					)
					(i32.const 1)
				)
				(i32.const 16)
			)
		)
	)

	(func $nested-br_table-value-index (export "nested-br_table-value-index") (param i32) (result i32)
		(i32.add
			(i32.const 1)
			(block (result i32)
				(drop (i32.const 2))
				(br_table 0
					(i32.const 4)
					(block (result i32)
						(drop (br_if 1 (i32.const 8) (local.get 0))) (i32.const 1)
					)
				)
				(i32.const 16)
			)
		)
	)

	(func (export "_start") (param i32)

		(call $type-i32)
		(call $type-i64)
		(call $type-f32)
		(call $type-f64)

		(if (i32.ne (call $type-i32-value) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $type-i64-value) (i64.const 2)) (then     
			(call $proc_exit (i32.const 2))
		))

		(if (f32.ne (call $type-f32-value) (f32.const 3)) (then     
			(call $proc_exit (i32.const 3))
		))

		(if (f64.ne (call $type-f64-value) (f64.const 4)) (then     
			(call $proc_exit (i32.const 4))
		))

		(if (i32.ne (call $as-block-first (i32.const 0)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 5))
		))

		(if (i32.ne (call $as-block-first (i32.const 1)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 6))
		))
		
		(if (i32.ne (call $as-block-mid (i32.const 0)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 7))
		))

		(if (i32.ne (call $as-block-mid (i32.const 1)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 8))
		))

		(call $as-block-last (i32.const 0))
		(call $as-block-last (i32.const 1))

		(if (i32.ne (call $as-block-first-value (i32.const 0)) (i32.const 11)) (then     
			(call $proc_exit (i32.const 9))
		))

		(if (i32.ne (call $as-block-first-value (i32.const 1)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 10))
		))

		(if (i32.ne (call $as-block-mid-value (i32.const 0)) (i32.const 21)) (then     
			(call $proc_exit (i32.const 11))
		))

		(if (i32.ne (call $as-block-mid-value (i32.const 1)) (i32.const 20)) (then     
			(call $proc_exit (i32.const 12))
		))

		(if (i32.ne (call $as-block-last-value (i32.const 0)) (i32.const 11)) (then     
			(call $proc_exit (i32.const 13))
		))
		
		(if (i32.ne (call $as-block-last-value (i32.const 1)) (i32.const 11)) (then     
			(call $proc_exit (i32.const 14))
		))

		(if (i32.ne (call $as-loop-first (i32.const 0)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 15))
		))

		(if (i32.ne (call $as-loop-first (i32.const 1)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 16))
		))

		(if (i32.ne (call $as-loop-mid (i32.const 0)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 17))
		))

		(if (i32.ne (call $as-loop-mid (i32.const 1)) (i32.const 4)) (then     
			(call $proc_exit (i32.const 18))
		))

		(call $as-loop-last (i32.const 0))
		(call $as-loop-last (i32.const 1))
		;; (call $as-br-value (i32.const 1))

		(call $as-br_if-cond)
		;; (call $as-br_if-value (i32.const 1))

		(if (i32.ne (call $as-br_if-value-cond (i32.const 0)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 19))
		))

		(if (i32.ne (call $as-br_if-value-cond (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 20))
		))

		(call $as-br_table-index)

		(call $as-br_table-value (i32.const 1))
		drop
		drop

		(call $as-br_table-value-index (i32.const 1))
		drop
		drop

		(call $as-return-value (i32.const 1))
		drop
		drop

		(if (i32.ne (call $as-if-cond (i32.const 0)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 21))
		))

		(if (i32.ne (call $as-if-cond (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 22))
		))

		(call $as-if-then (i32.const 0) (i32.const 0))
		(call $as-if-then (i32.const 4) (i32.const 0))
		(call $as-if-then (i32.const 0) (i32.const 1))
		(call $as-if-then (i32.const 4) (i32.const 1))
		(call $as-if-else (i32.const 0) (i32.const 0))
		(call $as-if-else (i32.const 3) (i32.const 0))
		(call $as-if-else (i32.const 0) (i32.const 1))
		(call $as-if-else (i32.const 3) (i32.const 1))

		(if (i32.ne (call $as-select-first (i32.const 0)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 23))
		))

		(if (i32.ne (call $as-select-first (i32.const 1)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 24))
		))
		
		(if (i32.ne (call $as-select-second (i32.const 0)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 25))
		))
		
		(if (i32.ne (call $as-select-second (i32.const 1)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 26))
		))

		(call $as-select-cond (i32.const 3))
		drop
		drop

		(call $as-call-first (i32.const 12))
		drop
		drop

		(call $as-call-mid (i32.const 13))
		drop
		drop

		(call $as-call-last (i32.const 14))
		drop
		drop

		(call $as-call_indirect-func (i32.const 4))
		drop
		drop

		(call $as-call_indirect-first (i32.const 4))
		drop
		drop

		(call $as-call_indirect-mid (i32.const 4))
		drop
		drop

		(call $as-call_indirect-last (i32.const 4))
		drop
		drop

		(if (i32.ne (call $as-local.set-value (i32.const 0)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 27))
		))

		(if (i32.ne (call $as-local.set-value (i32.const 1)) (i32.const 17)) (then     
			(call $proc_exit (i32.const 28))
		))

		(if (i32.ne (call $as-local.tee-value (i32.const 0)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 29))
		))

		(if (i32.ne (call $as-local.tee-value (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 30))
		))

		(if (i32.ne (call $as-global.set-value (i32.const 0)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 31))
		))

		(if (i32.ne (call $as-global.set-value (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 32))
		))

		(call $as-load-address (i32.const 1))
		drop
		drop

		(call $as-loadN-address (i32.const 30))
		drop
		drop

		(call $as-store-address (i32.const 31))
		drop
		drop

		(call $as-storeN-address (i32.const 32))
		drop
		drop

		(call $as-storeN-value (i32.const 33))
		drop
		drop

		(call $as-unary-operand (f64.const 1.0))
		drop
		drop

		(call $as-unary-operand (f64.const 1.0))
		drop
		drop

		(call $as-binary-left (i32.const 1))
		drop
		drop

		(call $as-binary-right (i32.const 1))
		drop
		drop

		(call $as-test-operand (i32.const 0))
		drop
		drop

		(call $as-compare-left (i32.const 1))
		drop
		drop

		(call $as-compare-right (i32.const 1))
		drop
		drop
		
		(call $as-memory.grow-size (i32.const 1))
		drop
		drop

		(if (i32.ne (call $nested-block-value (i32.const 0)) (i32.const 21)) (then     
			(call $proc_exit (i32.const 33))
		))

		(if (i32.ne (call $nested-block-value (i32.const 1)) (i32.const 9)) (then     
			(call $proc_exit (i32.const 34))
		))

		(if (i32.ne (call $nested-br-value (i32.const 0)) (i32.const 5)) (then     
			(call $proc_exit (i32.const 35))
		))

		(if (i32.ne (call $nested-br-value (i32.const 1)) (i32.const 9)) (then     
			(call $proc_exit (i32.const 36))
		))

		(if (i32.ne (call $nested-br_if-value (i32.const 0)) (i32.const 5)) (then     
			(call $proc_exit (i32.const 37))
		))

		(if (i32.ne (call $nested-br_if-value (i32.const 1)) (i32.const 9)) (then     
			(call $proc_exit (i32.const 38))
		))

		(if (i32.ne (call $nested-br_if-value-cond (i32.const 0)) (i32.const 5)) (then     
			(call $proc_exit (i32.const 39))
		))
		
		(if (i32.ne (call $nested-br_if-value-cond (i32.const 1)) (i32.const 9)) (then     
			(call $proc_exit (i32.const 40))
		))

		(if (i32.ne (call $nested-br_table-value (i32.const 0)) (i32.const 5)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i32.ne (call $nested-br_table-value (i32.const 1)) (i32.const 9)) (then     
			(call $proc_exit (i32.const 42))
		))

		(if (i32.ne (call $nested-br_table-value-index (i32.const 0)) (i32.const 5)) (then     
			(call $proc_exit (i32.const 43))
		))

		(if (i32.ne (call $nested-br_table-value-index (i32.const 1)) (i32.const 9)) (then     
			(call $proc_exit (i32.const 44))
		))

		(call $proc_exit (i32.const 0))

	)
)
