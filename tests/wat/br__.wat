;; Test `br` operator

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))

	;; Auxiliary definition
	(func $dummy)

	(func $type-i32-value (export "type-i32-value") (result i32)
		(block (result i32) (i32.ctz (br 0 (i32.const 1))))
	)

	(func $type-i64-value (export "type-i64-value") (result i64)
		(block (result i64) (i64.ctz (br 0 (i64.const 2))))
	)

	(func $type-f32-value (export "type-f32-value") (result f32)
		(block (result f32) (f32.neg (br 0 (f32.const 3))))
	)

	(func $type-f64-value (export "type-f64-value") (result f64)
		(block (result f64) (f64.neg (br 0 (f64.const 4))))
	)

	;; error: multiple result values not currently supported.
	;; (func $type-f64-f64-value (export "type-f64-f64-value") (result f64 f64)
	;; 	(block (result f64 f64)
	;; 		(f64.add (br 0 (f64.const 4) (f64.const 5))) (f64.const 6)
	;; 	)
	;; )

	(func $as-block-first (export "as-block-first")
		(block (br 0) (call $dummy))
	)

	(func $as-block-mid (export "as-block-mid")
		(block (call $dummy) (br 0) (call $dummy))
	)

	(func $as-block-last (export "as-block-last")
		(block (nop) (call $dummy) (br 0))
	)

	(func $as-block-value (export "as-block-value") (result i32)
		(block (result i32) (nop) (call $dummy) (br 0 (i32.const 2)))
	)

	(func $as-loop-first (export "as-loop-first") (result i32)
		(block (result i32) (loop (result i32) (br 1 (i32.const 3)) (i32.const 2)))
	)

	(func $as-loop-mid (export "as-loop-mid") (result i32)
		(block (result i32)
			(loop (result i32) (call $dummy) (br 1 (i32.const 4)) (i32.const 2))
		)
	)

	(func $as-loop-last (export "as-loop-last") (result i32)
		(block (result i32)
			(loop (result i32) (nop) (call $dummy) (br 1 (i32.const 5)))
		)
	)

	(func $as-br-value (export "as-br-value") (result i32)
		(block (result i32) (br 0 (br 0 (i32.const 9))))
	)

	(func $as-br_if-cond (export "as-br_if-cond")
		(block (br_if 0 (br 0)))
	)
	(func $as-br_if-value (export "as-br_if-value") (result i32)
		(block (result i32)
			(drop (br_if 0 (br 0 (i32.const 8)) (i32.const 1))) (i32.const 7)
		)
	)
	(func $as-br_if-value-cond (export "as-br_if-value-cond") (result i32)
		(block (result i32)
			(drop (br_if 0 (i32.const 6) (br 0 (i32.const 9)))) (i32.const 7)
		)
	)

	(func $as-br_table-index (export "as-br_table-index")
		(block (br_table 0 0 0 (br 0)))
	)
	(func $as-br_table-value (export "as-br_table-value") (result i32)
		(block (result i32)
			(br_table 0 0 0 (br 0 (i32.const 10)) (i32.const 1)) (i32.const 7)
		)
	)
	(func $as-br_table-value-index (export "as-br_table-value-index") (result i32)
		(block (result i32)
			(br_table 0 0 (i32.const 6) (br 0 (i32.const 11))) (i32.const 7)
		)
	)

	(func $as-return-value (export "as-return-value") (result i64)
		(block (result i64) (return (br 0 (i64.const 7))))
	)

	;; error: multiple result values not currently supported.
	;; (func $as-return-values (export "as-return-values") (result i32 i64)
	;; 	(i32.const 2)
	;; 	(block (result i64) (return (br 0 (i32.const 1) (i64.const 7))))
	;; )

	(func $as-if-cond (export "as-if-cond") (result i32)
		(block (result i32)
			(if (result i32) (br 0 (i32.const 2))
				(then (i32.const 0))
				(else (i32.const 1))
			)
		)
	)
	(func $as-if-then (export "as-if-then") (param i32 i32) (result i32)
		(block (result i32)
			(if (result i32) (local.get 0)
				(then (br 1 (i32.const 3)))
				(else (local.get 1))
			)
		)
	)
	(func $as-if-else (export "as-if-else") (param i32 i32) (result i32)
		(block (result i32)
			(if (result i32) (local.get 0)
				(then (local.get 1))
				(else (br 1 (i32.const 4)))
			)
		)
	)

	(func $f (param i32 i32 i32) (result i32) (i32.const -1))
	(func $as-call-first (export "as-call-first") (result i32)
		(block (result i32)
			(call $f (br 0 (i32.const 12)) (i32.const 2) (i32.const 3))
		)
	)
	(func $as-call-mid (export "as-call-mid") (result i32)
		(block (result i32)
			(call $f (i32.const 1) (br 0 (i32.const 13)) (i32.const 3))
		)
	)
	(func $as-call-last (export "as-call-last") (result i32)
		(block (result i32)
			(call $f (i32.const 1) (i32.const 2) (br 0 (i32.const 14)))
		)
	)
	(func $as-call-all (export "as-call-all") (result i32)
		(block (result i32) (call $f (br 0 (i32.const 15))))
	)

	(type $sig (func (param i32 i32 i32) (result i32)))
	(table funcref (elem $f))
	(func $as-call_indirect-func (export "as-call_indirect-func") (result i32)
		(block (result i32)
			(call_indirect (type $sig)
				(br 0 (i32.const 20))
				(i32.const 1) (i32.const 2) (i32.const 3)
			)
		)
	)
	(func $as-call_indirect-first (export "as-call_indirect-first") (result i32)
		(block (result i32)
			(call_indirect (type $sig)
				(i32.const 0)
				(br 0 (i32.const 21)) (i32.const 2) (i32.const 3)
			)
		)
	)
	(func $as-call_indirect-mid (export "as-call_indirect-mid") (result i32)
		(block (result i32)
			(call_indirect (type $sig)
				(i32.const 0)
				(i32.const 1) (br 0 (i32.const 22)) (i32.const 3)
			)
		)
	)
	(func $as-call_indirect-last (export "as-call_indirect-last") (result i32)
		(block (result i32)
			(call_indirect (type $sig)
				(i32.const 0)
				(i32.const 1) (i32.const 2) (br 0 (i32.const 23))
			)
		)
	)
	(func $as-call_indirect-all (export "as-call_indirect-all") (result i32)
		(block (result i32) (call_indirect (type $sig) (br 0 (i32.const 24))))
	)

	(func $as-local.set-value (export "as-local.set-value") (result i32) (local f32)
		(block (result i32) (local.set 0 (br 0 (i32.const 17))) (i32.const -1))
	)
	(func $as-local.tee-value (export "as-local.tee-value") (result i32) (local i32)
		(block (result i32) (local.tee 0 (br 0 (i32.const 1))))
	)
	(global $a (mut i32) (i32.const 10))
	(func $as-global.set-value (export "as-global.set-value") (result i32)
		(block (result i32) (global.set $a (br 0 (i32.const 1))))
	)

	(memory 1)
	(func $as-load-address (export "as-load-address") (result f32)
		(block (result f32) (f32.load (br 0 (f32.const 1.7))))
	)
	(func $as-loadN-address (export "as-loadN-address") (result i64)
		(block (result i64) (i64.load8_s (br 0 (i64.const 30))))
	)

	(func $as-store-address (export "as-store-address") (result i32)
		(block (result i32)
			(f64.store (br 0 (i32.const 30)) (f64.const 7)) (i32.const -1)
		)
	)
	(func $as-store-value (export "as-store-value") (result i32)
		(block (result i32)
			(i64.store (i32.const 2) (br 0 (i32.const 31))) (i32.const -1)
		)
	)
	(func $as-store-both (export "as-store-both") (result i32)
		(block (result i32)
			(i64.store (br 0 (i32.const 32))) (i32.const -1)
		)
	)

	(func $as-storeN-address (export "as-storeN-address") (result i32)
		(block (result i32)
			(i32.store8 (br 0 (i32.const 32)) (i32.const 7)) (i32.const -1)
		)
	)
	
	(func $as-storeN-value (export "as-storeN-value") (result i32)
		(block (result i32)
			(i64.store16 (i32.const 2) (br 0 (i32.const 33))) (i32.const -1)
		)
	)

	(func $as-storeN-both (export "as-storeN-both") (result i32)
		(block (result i32)
			(i64.store16 (br 0 (i32.const 34))) (i32.const -1)
		)
	)

	(func $as-unary-operand (export "as-unary-operand") (result f32)
		(block (result f32) (f32.neg (br 0 (f32.const 3.4))))
	)

	(func $as-binary-left (export "as-binary-left") (result i32)
		(block (result i32) (i32.add (br 0 (i32.const 3)) (i32.const 10)))
	)

	(func $as-binary-right (export "as-binary-right") (result i64)
		(block (result i64) (i64.sub (i64.const 10) (br 0 (i64.const 45))))
	)

	(func $as-binary-both (export "as-binary-both") (result i32)
		(block (result i32) (i32.add (br 0 (i32.const 46))))
	)

	(func $as-test-operand (export "as-test-operand") (result i32)
		(block (result i32) (i32.eqz (br 0 (i32.const 44))))
	)

	(func $as-compare-left (export "as-compare-left") (result i32)
		(block (result i32) (f64.le (br 0 (i32.const 43)) (f64.const 10)))
	)
	(func $as-compare-right (export "as-compare-right") (result i32)
		(block (result i32) (f32.ne (f32.const 10) (br 0 (i32.const 42))))
	)
	(func $as-compare-both (export "as-compare-both") (result i32)
		(block (result i32) (f64.le (br 0 (i32.const 44))))
	)

	(func $as-convert-operand (export "as-convert-operand") (result i32)
		(block (result i32) (i32.wrap_i64 (br 0 (i32.const 41))))
	)

	(func $as-memory.grow-size (export "as-memory.grow-size") (result i32)
		(block (result i32) (memory.grow (br 0 (i32.const 40))))
	)

	(func $nested-block-value (export "nested-block-value") (result i32)
		(i32.add
			(i32.const 1)
			(block (result i32)
				(call $dummy)
				(i32.add (i32.const 4) (br 0 (i32.const 8)))
			)
		)
	)

	(func $nested-br-value (export "nested-br-value") (result i32)
		(i32.add
			(i32.const 1)
			(block (result i32)
				(drop (i32.const 2))
				(drop
					(block (result i32)
						(drop (i32.const 4))
						(br 0 (br 1 (i32.const 8)))
					)
				)
				(i32.const 16)
			)
		)
	)

	(func $nested-br_if-value (export "nested-br_if-value") (result i32)
		(i32.add
			(i32.const 1)
			(block (result i32)
				(drop (i32.const 2))
				(drop
					(block (result i32)
						(drop (i32.const 4))
						(drop (br_if 0 (br 1 (i32.const 8)) (i32.const 1)))
						(i32.const 32)
					)
				)
				(i32.const 16)
			)
		)
	)

	(func $nested-br_if-value-cond (export "nested-br_if-value-cond") (result i32)
		(i32.add
			(i32.const 1)
			(block (result i32)
				(drop (i32.const 2))
				(drop (br_if 0 (i32.const 4) (br 0 (i32.const 8))))
				(i32.const 16)
			)
		)
	)

	(func $nested-br_table-value (export "nested-br_table-value") (result i32)
		(i32.add
			(i32.const 1)
			(block (result i32)
				(drop (i32.const 2))
				(drop
					(block (result i32)
						(drop (i32.const 4))
						(br_table 0 (br 1 (i32.const 8)) (i32.const 1))
					)
				)
				(i32.const 16)
			)
		)
	)

	(func $nested-br_table-value-index (export "nested-br_table-value-index") (result i32)
		(i32.add
			(i32.const 1)
			(block (result i32)
				(drop (i32.const 2))
				(br_table 0 (i32.const 4) (br 0 (i32.const 8)))
				(i32.const 16)
			)
		)
	)

	(func (export "_start") (param i32)
		(call $type-i32-value (i32.const 1))
		drop
		drop
		(call $type-i64-value (i64.const 2))
		drop
		drop
		(call $type-f32-value (f32.const 3))
		drop
		drop
		(call $type-f64-value (f64.const 4))
		drop
		drop
		;; (call $type-f64-f64-value (f64.const 4) (f64.const 5))
		;; drop
		;; drop

		(call $as-block-first)
		(call $as-block-mid)
		(call $as-block-last)
		(call $as-block-value (i32.const 2))
		drop
		drop

		(call $as-loop-first (i32.const 3))
		drop
		drop
		(call $as-loop-mid (i32.const 4))
		drop
		drop
		(call $as-loop-last (i32.const 5))
		drop
		drop

		(call $as-br-value (i32.const 9))
		drop
		drop

		(call $as-br_if-cond)
		(call $as-br_if-value (i32.const 8))
		drop
		drop
		(call $as-br_if-value-cond (i32.const 9))
		drop
		drop

		(call $as-br_table-index)
		(call $as-br_table-value (i32.const 10))
		drop
		drop
		(call $as-br_table-value-index (i32.const 11))
		drop
		drop

		(call $as-return-value (i64.const 7))
		drop
		drop
		;; (call $as-return-values (i32.const 2) (i64.const 7))
		;; drop
		;; drop

		(call $as-if-cond (i32.const 2))
		drop
		drop

		(if (i32.ne (call $as-if-then (i32.const 1) (i32.const 6)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-if-then (i32.const 0) (i32.const 6)) (i32.const 6)) (then     
			(call $proc_exit (i32.const 2))
		))
		
		(if (i32.ne (call $as-if-else (i32.const 0) (i32.const 6)) (i32.const 4)) (then     
			(call $proc_exit (i32.const 3))
		))

		(if (i32.ne (call $as-if-else (i32.const 1) (i32.const 6)) (i32.const 6)) (then     
			(call $proc_exit (i32.const 4))
		))

		(call $as-call-first (i32.const 12))
		drop
		drop
		(call $as-call-mid (i32.const 13))
		drop
		drop
		(call $as-call-last (i32.const 14))
		drop
		drop
		(call $as-call-all (i32.const 15))
		drop
		drop

		(call $as-call_indirect-func (i32.const 20))
		drop
		drop
		(call $as-call_indirect-first (i32.const 21))
		drop
		drop
		(call $as-call_indirect-mid (i32.const 22))
		drop
		drop
		(call $as-call_indirect-last (i32.const 23))
		drop
		drop
		(call $as-call_indirect-all (i32.const 24))
		drop
		drop

		(call $as-local.set-value (i32.const 17))
		drop
		drop
		(call $as-local.tee-value (i32.const 1))
		drop
		drop
		(call $as-global.set-value (i32.const 1))
		drop
		drop

		(call $as-load-address (f32.const 1.7))
		drop
		drop
		(call $as-loadN-address (i64.const 30))
		drop
		drop

		(call $as-store-address (i32.const 30))
		drop
		drop
		(call $as-store-value (i32.const 31))
		drop
		drop
		(call $as-store-both (i32.const 32))
		drop
		drop
		(call $as-storeN-address (i32.const 32))
		drop
		drop
		(call $as-storeN-value (i32.const 33))
		drop
		drop
		(call $as-storeN-both (i32.const 34))
		drop
		drop

		(call $as-unary-operand (f32.const 3.4))
		drop
		drop

		(call $as-binary-left (i32.const 3))
		drop
		drop
		(call $as-binary-right (i64.const 45))
		drop
		drop
		(call $as-binary-both (i32.const 46))
		drop
		drop

		(call $as-test-operand (i32.const 44))
		drop
		drop

		(call $as-compare-left (i32.const 43))
		drop
		drop
		(call $as-compare-right (i32.const 42))
		drop
		drop
		(call $as-compare-both (i32.const 44))
		drop
		drop

		(call $as-convert-operand (i32.const 41))
		drop
		drop

		(call $as-memory.grow-size (i32.const 40))
		drop
		drop

		(call $nested-block-value (i32.const 9))
		drop
		drop
		(call $nested-br-value (i32.const 9))
		drop
		drop
		(call $nested-br_if-value (i32.const 9))
		drop
		drop
		(call $nested-br_if-value-cond (i32.const 9))
		drop
		drop
		(call $nested-br_table-value (i32.const 9))
		drop
		drop
		(call $nested-br_table-value-index (i32.const 9))
		drop
		drop

			
		(call $proc_exit (i32.const 0))
	)
)

