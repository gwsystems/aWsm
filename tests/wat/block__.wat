;; Derived from https://github.com/WebAssembly/testsuite/blob/main/block.wast

(module
  (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
  ;; Auxiliary definition
  (memory 1)
  (export "memory" (memory 0))

  (func $dummy)

  (func $empty (export "empty")
	(block)
	(block $l)
  )

  (func $singular (export "singular") (result i32)
	(block (nop))
	(block (result i32) (i32.const 7))
  )

;; error: multiple block results not currently supported.
;;   (func $multi (export "multi") (result i32)
;;     (block (call $dummy) (call $dummy) (call $dummy) (call $dummy))
;;     (block (result i32)
;;       (call $dummy) (call $dummy) (call $dummy) (i32.const 7) (call $dummy)
;;     )
;;     (drop)
;;     (block (result i32 i64 i32)
;;       (call $dummy) (call $dummy) (call $dummy) (i32.const 8) (call $dummy)
;;       (call $dummy) (call $dummy) (call $dummy) (i64.const 7) (call $dummy)
;;       (call $dummy) (call $dummy) (call $dummy) (i32.const 9) (call $dummy)
;;     )
;;     (drop) (drop)


  (func $nested (export "nested") (result i32)
	(block (result i32)
	  (block (call $dummy) (block) (nop))
	  (block (result i32) (call $dummy) (i32.const 9))
	)
  )

  (func $deep (export "deep") (result i32)
	(block (result i32) (block (result i32)
	  (block (result i32) (block (result i32)
		(block (result i32) (block (result i32)
		  (block (result i32) (block (result i32)
			(block (result i32) (block (result i32)
			  (block (result i32) (block (result i32)
				(block (result i32) (block (result i32)
				  (block (result i32) (block (result i32)
					(block (result i32) (block (result i32)
					  (block (result i32) (block (result i32)
						(block (result i32) (block (result i32)
						  (block (result i32) (block (result i32)
							(block (result i32) (block (result i32)
							  (block (result i32) (block (result i32)
								(block (result i32) (block (result i32)
								  (block (result i32) (block (result i32)
									(block (result i32) (block (result i32)
									  (block (result i32) (block (result i32)
										(block (result i32) (block (result i32)
										  (call $dummy) (i32.const 150)
										))
									  ))
									))
								  ))
								))
							  ))
							))
						  ))
						))
					  ))
					))
				  ))
				))
			  ))
			))
		  ))
		))
	  ))
	))
  )

  (func $as-select-first (export "as-select-first") (result i32)
	(select (block (result i32) (i32.const 1)) (i32.const 2) (i32.const 3))
  )
  (func $as-select-mid (export "as-select-mid") (result i32)
	(select (i32.const 2) (block (result i32) (i32.const 1)) (i32.const 3))
  )
  (func $as-select-last (export "as-select-last") (result i32)
	(select (i32.const 2) (i32.const 3) (block (result i32) (i32.const 1)))
  )

  (func $as-loop-first (export "as-loop-first") (result i32)
	(loop (result i32) (block (result i32) (i32.const 1)) (call $dummy) (call $dummy))
  )
  (func $as-loop-mid (export "as-loop-mid") (result i32)
	(loop (result i32) (call $dummy) (block (result i32) (i32.const 1)) (call $dummy))
  )
  (func $as-loop-last (export "as-loop-last") (result i32)
	(loop (result i32) (call $dummy) (call $dummy) (block (result i32) (i32.const 1)))
  )

  (func $as-if-condition (export "as-if-condition")
	(block (result i32) (i32.const 1)) (if (then (call $dummy)))
  )
  (func $as-if-then (export "as-if-then") (result i32)
	(if (result i32) (i32.const 1) (then (block (result i32) (i32.const 1))) (else (i32.const 2)))
  )
  (func $as-if-else (export "as-if-else") (result i32)
	(if (result i32) (i32.const 1) (then (i32.const 2)) (else (block (result i32) (i32.const 1))))
  )

  (func $as-br_if-first (export "as-br_if-first") (result i32)
	(block (result i32) (br_if 0 (block (result i32) (i32.const 1)) (i32.const 2)))
  )
  (func $as-br_if-last (export "as-br_if-last") (result i32)
	(block (result i32) (br_if 0 (i32.const 2) (block (result i32) (i32.const 1))))
  )

  (func $as-br_table-first (export "as-br_table-first") (result i32)
	(block (result i32) (block (result i32) (i32.const 1)) (i32.const 2) (br_table 0 0))
  )
  (func $as-br_table-last (export "as-br_table-last") (result i32)
	(block (result i32) (i32.const 2) (block (result i32) (i32.const 1)) (br_table 0 0))
  )

  (func $func (param i32 i32) (result i32) (local.get 0))
  (type $check (func (param i32 i32) (result i32)))
  (table funcref (elem $func))
  (func $as-call_indirect-first (export "as-call_indirect-first") (result i32)
	(block (result i32)
	  (call_indirect (type $check)
		(block (result i32) (i32.const 1)) (i32.const 2) (i32.const 0)
	  )
	)
  )
  (func $as-call_indirect-mid (export "as-call_indirect-mid") (result i32)
	(block (result i32)
	  (call_indirect (type $check)
		(i32.const 2) (block (result i32) (i32.const 1)) (i32.const 0)
	  )
	)
  )
  (func $as-call_indirect-last (export "as-call_indirect-last") (result i32)
	(block (result i32)
	  (call_indirect (type $check)
		(i32.const 1) (i32.const 2) (block (result i32) (i32.const 0))
	  )
	)
  )

  (func $as-store-first (export "as-store-first")
	(block (result i32) (i32.const 1)) (i32.const 1) (i32.store)
  )
  (func $as-store-last (export "as-store-last")
	(i32.const 10) (block (result i32) (i32.const 1)) (i32.store)
  )

  (func $as-memory.grow-value (export "as-memory.grow-value") (result i32)
	(memory.grow (block (result i32) (i32.const 1)))
  )

  (func $f (param i32) (result i32) (local.get 0))

  (func $as-call-value (export "as-call-value") (result i32)
	(call $f (block (result i32) (i32.const 1)))
  )
  (func $as-return-value (export "as-return-value") (result i32)
	(block (result i32) (i32.const 1)) (return)
  )
  (func $as-drop-operand (export "as-drop-operand")
	(drop (block (result i32) (i32.const 1)))
  )
  (func $as-br-value (export "as-br-value") (result i32)
	(block (result i32) (br 0 (block (result i32) (i32.const 1))))
  )
  (func $as-local.set-value (export "as-local.set-value") (result i32)
	(local i32) (local.set 0 (block (result i32) (i32.const 1))) (local.get 0)
  )
  (func $as-local.tee-value (export "as-local.tee-value") (result i32)
	(local i32) (local.tee 0 (block (result i32) (i32.const 1)))
  )
  (global $a (mut i32) (i32.const 10))
  (func $as-global.set-value (export "as-global.set-value") (result i32)
	(global.set $a (block (result i32) (i32.const 1)))
	(global.get $a)
  )

  (func $as-load-operand (export "as-load-operand") (result i32)
	(i32.load (block (result i32) (i32.const 1)))
  )

  (func $as-unary-operand (export "as-unary-operand") (result i32)
	(i32.ctz (block (result i32) (call $dummy) (i32.const 13)))
  )
  (func $as-binary-operand (export "as-binary-operand") (result i32)
	(i32.mul
	  (block (result i32) (call $dummy) (i32.const 3))
	  (block (result i32) (call $dummy) (i32.const 4))
	)
  )
  (func $as-test-operand (export "as-test-operand") (result i32)
	(i32.eqz (block (result i32) (call $dummy) (i32.const 13)))
  )
  (func $as-compare-operand (export "as-compare-operand") (result i32)
	(f32.gt
	  (block (result f32) (call $dummy) (f32.const 3))
	  (block (result f32) (call $dummy) (f32.const 3))
	)
  )

;;   error: multiple block results not currently supported.
;;   (func $as-binary-operands (export "as-binary-operands") (result i32)
;;     (i32.mul
;;       (block (result i32 i32)
;;         (call $dummy) (i32.const 3) (call $dummy) (i32.const 4)
;;       )
;;     )


;;    error: multiple block results not currently supported.
;;   (func $as-compare-operands (export "as-compare-operands") (result i32)
;;     (f32.gt
;;       (block (result f32 f32)
;;         (call $dummy) (f32.const 3) (call $dummy) (f32.const 3)
;;       )
;;     )


;; error: multiple block results not currently supported.
;;   (func $as-mixed-operands (export "as-mixed-operands") (result i32)
;;     (block (result i32 i32)
;;       (call $dummy) (i32.const 3) (call $dummy) (i32.const 4)
;;     )
;;     (i32.const 5)
;;     (i32.add)
;;     (i32.mul)


  (func $break-bare (export "break-bare") (result i32)
	(block (br 0) (unreachable))
	(block (br_if 0 (i32.const 1)) (unreachable))
	(block (br_table 0 (i32.const 0)) (unreachable))
	(block (br_table 0 0 0 (i32.const 1)) (unreachable))
	(i32.const 19)
  )
  (func $break-value (export "break-value") (result i32)
	(block (result i32) (br 0 (i32.const 18)) (i32.const 19))
  )

;;   error: multiple result values not currently supported.
;;   (func $break-multi-value (export "break-multi-value") (result i32 i32 i64)
;;     (block (result i32 i32 i64)
;;       (br 0 (i32.const 18) (i32.const -18) (i64.const 18))
;;       (i32.const 19) (i32.const -19) (i64.const 19)
;;     )


  (func $break-repeated (export "break-repeated") (result i32)
	(block (result i32)
	  (br 0 (i32.const 18))
	  (br 0 (i32.const 19))
	  (drop (br_if 0 (i32.const 20) (i32.const 0)))
	  (drop (br_if 0 (i32.const 20) (i32.const 1)))
	  (br 0 (i32.const 21))
	  (br_table 0 (i32.const 22) (i32.const 4))
	  (br_table 0 0 0 (i32.const 23) (i32.const 1))
	  (i32.const 21)
	)
  )

;; error: block params not currently supported.
;;   (func $param (export "param") (result i32)
;;     (i32.const 1)
;;     (block (param i32) (result i32)
;;       (i32.const 2)
;;       (i32.add)
;;     )


;;   error: block params not currently supported.
;;   (func $params (export "params") (result i32)
;;     (i32.const 1)
;;     (i32.const 2)
;;     (block (param i32 i32) (result i32)
;;       (i32.add)
;;     )


;;   error: block params not currently supported.
;;   error: multiple block results not currently supported.
;;   (func $params-id (export "params-id") (result i32)
;;     (i32.const 1)
;;     (i32.const 2)
;;     (block (param i32 i32) (result i32 i32))
;;     (i32.add)


;; error: block params not currently supported.
;;   (func $param-break (export "param-break") (result i32)
;;     (i32.const 1)
;;     (block (param i32) (result i32)
;;       (i32.const 2)
;;       (i32.add)
;;       (br 0)
;;     )


;; error: block params not currently supported.
;;   (func $params-break (export "params-break") (result i32)
;;     (i32.const 1)
;;     (i32.const 2)
;;     (block (param i32 i32) (result i32)
;;       (i32.add)
;;       (br 0)
;;     )


;; error: block params not currently supported.
;; error: multiple block results not currently supported.
;;   (func $params-id-break (export "params-id-break") (result i32)
;;     (i32.const 1)
;;     (i32.const 2)
;;     (block (param i32 i32) (result i32 i32) (br 0))
;;     (i32.add)


  (func $effects (export "effects") (result i32)
	(local i32)
	(block
	  (local.set 0 (i32.const 1))
	  (local.set 0 (i32.mul (local.get 0) (i32.const 3)))
	  (local.set 0 (i32.sub (local.get 0) (i32.const 5)))
	  (local.set 0 (i32.mul (local.get 0) (i32.const 7)))
	  (br 0)
	  (local.set 0 (i32.mul (local.get 0) (i32.const 100)))
	)
	(i32.eq (local.get 0) (i32.const -14))
  )

  (type $block-sig-1 (func))
  (type $block-sig-2 (func (result i32)))
  (type $block-sig-3 (func (param $x i32)))
  (type $block-sig-4 (func (param i32 f64 i32) (result i32 f64 i32)))

;; error: block params not currently supported.
;; error: multiple block results not currently supported.
;;   (func $type-use (export "type-use")
;;     (block (type $block-sig-1))
;;     (block (type $block-sig-2) (i32.const 0))
;;     (block (type $block-sig-3) (drop))
;;     (i32.const 0) (f64.const 0) (i32.const 0)
;;     (block (type $block-sig-4))
;;     (drop) (drop) (drop)
;;     (block (type $block-sig-2) (result i32) (i32.const 0))
;;     (block (type $block-sig-3) (param i32) (drop))
;;     (i32.const 0) (f64.const 0) (i32.const 0)
;;     (block (type $block-sig-4)
;;       (param i32) (param f64 i32) (result i32 f64) (result i32)
;;     )
;;     (drop) (drop) (drop)


	(func (export "_start") (param i32)
		(call $empty)
  		(call $singular (i32.const 7))
  		;; (call $multi (i32.const 8))
  		(call $nested (i32.const 9))
		drop
		drop
		drop
		drop
  		(call $deep (i32.const 150))
		drop
		drop
  		(call $as-select-first (i32.const 1))
		drop
		drop
  		(call $as-select-mid (i32.const 2))
		drop
		drop
  		(call $as-select-last (i32.const 2))
		drop
		drop
  		(call $as-loop-first (i32.const 1))
		drop
		drop
  		(call $as-loop-mid (i32.const 1))
		drop
		drop
  		(call $as-loop-last (i32.const 1))
		drop
		drop
  		(call $as-if-then (i32.const 1))
		drop
		drop
  		(call $as-if-else (i32.const 2))
		drop
		drop
		(call $as-br_if-first (i32.const 1))
		drop
		drop
		(call $as-br_if-last (i32.const 2))
		drop
		drop
		(call $as-br_table-first (i32.const 1))
		drop
		drop
		(call $as-br_table-last (i32.const 2))
		drop
		drop
		(call $as-call_indirect-first (i32.const 1))
		drop
		drop
		(call $as-call_indirect-mid (i32.const 2))
		drop
		drop
		(call $as-call_indirect-last (i32.const 1))
		drop
		drop
  		(call $as-store-first)
  		(call $as-store-last)

		(call $as-memory.grow-value (i32.const 1))
		drop
		drop
		(call $as-call-value (i32.const 1))
		drop
		drop
		(call $as-return-value (i32.const 1))
		drop
		drop
		(call $as-drop-operand)
		(call $as-br-value (i32.const 1))
		drop
		drop
		(call $as-local.set-value (i32.const 1))
		drop
		drop
		(call $as-local.tee-value (i32.const 1))
		drop
		drop
		(call $as-global.set-value (i32.const 1))
		drop
		drop
		(call $as-load-operand (i32.const 1))
		drop
		drop
		(call $as-unary-operand (i32.const 0))
		drop
		drop
		(call $as-binary-operand (i32.const 12))
		drop
		drop
		(call $as-test-operand (i32.const 0))
		drop
		drop
		(call $as-compare-operand (i32.const 0))
		drop
		drop
		;; (call $as-binary-operands (i32.const 0))
		;; drop
		;; drop
		;; (call $as-compare-operands (i32.const 0))
		;; drop
		;; drop
		;; (call $as-mixed-operands (i32.const 27))
		;; drop
		;; drop

		(call $break-bare (i32.const 19))
		drop
		drop
		(call $break-value (i32.const 18))
		drop
		drop
		;; (call $break-multi-value (i32.const 18) (i32.const -18) (i64.const 18))
		;; drop
		;; drop
		(call $break-repeated (i32.const 18))
		drop
		drop
		;; (call $param (i32.const 3))
		;; drop
		;; drop
		;; (call $params (i32.const 3))
		;; drop
		;; drop
		;; (call $params-id (i32.const 3))
		;; drop
		;; drop
		;; (call $param-break (i32.const 3))
		;; drop
		;; drop
		;; (call $params-break (i32.const 3))
		;; drop
		;; drop
		;; (call $params-id-break (i32.const 3))
		;; drop
		;; drop

		(call $effects (i32.const 1))
		drop
		drop
		;; (call $type-use (i32.const 1))

  		(call $proc_exit (i32.const 0))
	)
)
