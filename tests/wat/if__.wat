;; Test `if` operator

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))

	;; Auxiliary definition
	(memory 1)

	(func $dummy)

	(func $empty (export "empty") (param i32)
		(if (local.get 0) (then))
		(if (local.get 0) (then) (else))
		(if $l (local.get 0) (then))
		(if $l (local.get 0) (then) (else))
	)

	(func $singular (export "singular") (param i32) (result i32)
		(if (local.get 0) (then (nop)))
		(if (local.get 0) (then (nop)) (else (nop)))
		(if (result i32) (local.get 0) (then (i32.const 7)) (else (i32.const 8)))
	)

	;;   (func (export "multi") (param i32) (result i32 i32)
	;;     (if (local.get 0) (then (call $dummy) (call $dummy) (call $dummy)))
	;;     (if (local.get 0) (then) (else (call $dummy) (call $dummy) (call $dummy)))
	;;     (if (result i32) (local.get 0)
	;;       (then (call $dummy) (call $dummy) (i32.const 8) (call $dummy))
	;;       (else (call $dummy) (call $dummy) (i32.const 9) (call $dummy))
	;;     )
	;;     (if (result i32 i64 i32) (local.get 0)
	;;       (then
	;;         (call $dummy) (call $dummy) (i32.const 1) (call $dummy)
	;;         (call $dummy) (call $dummy) (i64.const 2) (call $dummy)
	;;         (call $dummy) (call $dummy) (i32.const 3) (call $dummy)
	;;       )
	;;       (else
	;;         (call $dummy) (call $dummy) (i32.const -1) (call $dummy)
	;;         (call $dummy) (call $dummy) (i64.const -2) (call $dummy)
	;;         (call $dummy) (call $dummy) (i32.const -3) (call $dummy)
	;;       )
	;;     )
	;;     (drop) (drop)
	;;   )

	(func $nested (export "nested") (param i32 i32) (result i32)
		(if (result i32) (local.get 0) (then
			(if (local.get 1) (then (call $dummy) (block) (nop)))
			(if (local.get 1) (then) (else (call $dummy) (block) (nop)))
			(if (result i32) (local.get 1)
			(then (call $dummy) (i32.const 9))
			(else (call $dummy) (i32.const 10))
			)
		) (else
			(if (local.get 1) (then (call $dummy) (block) (nop)))
			(if (local.get 1) (then) (else (call $dummy) (block) (nop)))
			(if (result i32) (local.get 1)
			(then (call $dummy) (i32.const 10))
			(else (call $dummy) (i32.const 11))
			)
		))
	)

	(func $as-select-first (export "as-select-first") (param i32) (result i32)
		(select
			(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 1))
				(else (call $dummy) (i32.const 0))
			)
			(i32.const 2) (i32.const 3)
		)
	)

	(func $as-select-mid (export "as-select-mid") (param i32) (result i32)
		(select
			(i32.const 2)
			(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 1))
				(else (call $dummy) (i32.const 0))
			)
			(i32.const 3)
		)
	)

	(func $as-select-last (export "as-select-last") (param i32) (result i32)
		(select
			(i32.const 2) (i32.const 3)
			(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 1))
				(else (call $dummy) (i32.const 0))
			)
		)
	)

	(func $as-loop-first (export "as-loop-first") (param i32) (result i32)
		(loop (result i32)
			(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 1))
				(else (call $dummy) (i32.const 0))
			)
			(call $dummy) (call $dummy)
		)
	)

	(func $as-loop-mid (export "as-loop-mid") (param i32) (result i32)
		(loop (result i32)
			(call $dummy)
			(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 1))
				(else (call $dummy) (i32.const 0))
			)
			(call $dummy)
		)
	)

	(func $as-loop-last (export "as-loop-last") (param i32) (result i32)
		(loop (result i32)
			(call $dummy) (call $dummy)
			(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 1))
				(else (call $dummy) (i32.const 0))
			)
		)
	)

	(func $as-if-condition (export "as-if-condition") (param i32) (result i32)
		(if (result i32)
			(if (result i32) (local.get 0)
				(then (i32.const 1)) (else (i32.const 0))
			)
			(then (call $dummy) (i32.const 2))
			(else (call $dummy) (i32.const 3))
		)
	)

	(func $as-br_if-first (export "as-br_if-first") (param i32) (result i32)
		(block (result i32)
			(br_if 0
				(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 1))
				(else (call $dummy) (i32.const 0))
				)
				(i32.const 2)
			)
			(return (i32.const 3))
		)
	)

	(func $as-br_if-last (export "as-br_if-last") (param i32) (result i32)
		(block (result i32)
			(br_if 0
				(i32.const 2)
				(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 1))
				(else (call $dummy) (i32.const 0))
				)
			)
			(return (i32.const 3))
		)
	)

	(func $as-br_table-first (export "as-br_table-first") (param i32) (result i32)
		(block (result i32)
			(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 1))
				(else (call $dummy) (i32.const 0))
			)
			(i32.const 2)
			(br_table 0 0)
		)
	)

	(func $as-br_table-last (export "as-br_table-last") (param i32) (result i32)
		(block (result i32)
			(i32.const 2)
			(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 1))
				(else (call $dummy) (i32.const 0))
			)
			(br_table 0 0)
		)
	)

	(func $func (param i32 i32) (result i32) (local.get 0))

	(type $check (func (param i32 i32) (result i32)))

	(table funcref (elem $func))

	(func $as-call_indirect-first (export "as-call_indirect-first") (param i32) (result i32)
		(block (result i32)
		(call_indirect (type $check)
			(if (result i32) (local.get 0)
			(then (call $dummy) (i32.const 1))
			(else (call $dummy) (i32.const 0))
			)
			(i32.const 2) (i32.const 0)
		)
		)
	)

	(func $as-call_indirect-mid (export "as-call_indirect-mid") (param i32) (result i32)
		(block (result i32)
		(call_indirect (type $check)
			(i32.const 2)
			(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 1))
				(else (call $dummy) (i32.const 0))
			)
			(i32.const 0)
		)
		)
	)

	(func $as-call_indirect-last (export "as-call_indirect-last") (param i32) (result i32)
		(block (result i32)
		(call_indirect (type $check)
			(i32.const 2) (i32.const 0)
			(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 1))
				(else (call $dummy) (i32.const 0))
			)
		)
		)
	)

	(func $as-store-first (export "as-store-first") (param i32)
		(if (result i32) (local.get 0)
			(then (call $dummy) (i32.const 1))
			(else (call $dummy) (i32.const 0))
		)
		(i32.const 2)
		(i32.store)
	)

	(func $as-store-last (export "as-store-last") (param i32)
		(i32.const 2)
		(if (result i32) (local.get 0)
			(then (call $dummy) (i32.const 1))
			(else (call $dummy) (i32.const 0))
		)
		(i32.store)
	)

	(func $as-memory.grow-value (export "as-memory.grow-value") (param i32) (result i32)
		(memory.grow
			(if (result i32) (local.get 0)
				(then (i32.const 1))
				(else (i32.const 0))
			)
		)
	)

	(func $f (param i32) (result i32) (local.get 0))

	(func $as-call-value (export "as-call-value") (param i32) (result i32)
		(call $f
			(if (result i32) (local.get 0)
				(then (i32.const 1))
				(else (i32.const 0))
			)
		)
	)

	(func $as-return-value (export "as-return-value") (param i32) (result i32)
		(if (result i32) (local.get 0)
		(then (i32.const 1))
		(else (i32.const 0)))
		(return)
	)

	(func $as-drop-operand (export "as-drop-operand") (param i32)
		(drop
		(if (result i32) (local.get 0)
			(then (i32.const 1))
			(else (i32.const 0))
		)
		)
	)

	(func $as-br-value (export "as-br-value") (param i32) (result i32)
		(block (result i32)
			(br 0
				(if (result i32) (local.get 0)
					(then (i32.const 1))
					(else (i32.const 0))
				)
			)
		)
	)

	(func $as-local.set-value (export "as-local.set-value") (param i32) (result i32)
		(local i32)
		(local.set 0
			(if (result i32) (local.get 0)
				(then (i32.const 1))
				(else (i32.const 0))
			)
		)
		(local.get 0)
	)

	(func $as-local.tee-value (export "as-local.tee-value") (param i32) (result i32)
		(local.tee 0
			(if (result i32) (local.get 0)
				(then (i32.const 1))
				(else (i32.const 0))
			)
		)
	)

	(global $a (mut i32) (i32.const 10))

	(func $as-global.set-value (export "as-global.set-value") (param i32) (result i32)
		(global.set $a
			(if (result i32) (local.get 0)
				(then (i32.const 1))
				(else (i32.const 0))
			)
		) (global.get $a)
	)

	(func $as-load-operand (export "as-load-operand") (param i32) (result i32)
		(i32.load
			(if (result i32) (local.get 0)
				(then (i32.const 11))
				(else (i32.const 10))
			)
		)
	)

	(func $as-binary-operand (export "as-binary-operand") (param i32 i32) (result i32)
		(i32.mul
			(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 3))
				(else (call $dummy) (i32.const -3))
			)
			(if (result i32) (local.get 1)
				(then (call $dummy) (i32.const 4))
				(else (call $dummy) (i32.const -5))
			)
		)
	)

	(func $as-test-operand (export "as-test-operand") (param i32) (result i32)
		(i32.eqz
			(if (result i32) (local.get 0)
				(then (call $dummy) (i32.const 13))
				(else (call $dummy) (i32.const 0))
			)
		)
	)

	(func $as-compare-operand (export "as-compare-operand") (param i32 i32) (result i32)
		(f32.gt
			(if (result f32) (local.get 0)
				(then (call $dummy) (f32.const 3))
				(else (call $dummy) (f32.const -3))
			)
			(if (result f32) (local.get 1)
				(then (call $dummy) (f32.const 4))
				(else (call $dummy) (f32.const -4))
			)
		)
	)

	;;   (func (export "as-binary-operands") (param i32) (result i32)
	;;     (i32.mul
	;;       (if (result i32 i32) (local.get 0)
	;;         (then (call $dummy) (i32.const 3) (call $dummy) (i32.const 4))
	;;         (else (call $dummy) (i32.const 3) (call $dummy) (i32.const -4))
	;;       )
	;;     )
	;;   )

	;;   (func (export "as-compare-operands") (param i32) (result i32)
	;;     (f32.gt
	;;       (if (result f32 f32) (local.get 0)
	;;         (then (call $dummy) (f32.const 3) (call $dummy) (f32.const 3))
	;;         (else (call $dummy) (f32.const -2) (call $dummy) (f32.const -3))
	;;       )
	;;     )
	;;   )

	;;   (func (export "as-mixed-operands") (param i32) (result i32)
	;;     (if (result i32 i32) (local.get 0)
	;;       (then (call $dummy) (i32.const 3) (call $dummy) (i32.const 4))
	;;       (else (call $dummy) (i32.const -3) (call $dummy) (i32.const -4))
	;;     )
	;;     (i32.const 5)
	;;     (i32.add)
	;;     (i32.mul)
	;;   )

	(func $break-bare (export "break-bare") (result i32)
		(if (i32.const 1) (then (br 0) (unreachable)))
		(if (i32.const 1) (then (br 0) (unreachable)) (else (unreachable)))
		(if (i32.const 0) (then (unreachable)) (else (br 0) (unreachable)))
		(if (i32.const 1) (then (br_if 0 (i32.const 1)) (unreachable)))
		(if (i32.const 1) (then (br_if 0 (i32.const 1)) (unreachable)) (else (unreachable)))
		(if (i32.const 0) (then (unreachable)) (else (br_if 0 (i32.const 1)) (unreachable)))
		(if (i32.const 1) (then (br_table 0 (i32.const 0)) (unreachable)))
		(if (i32.const 1) (then (br_table 0 (i32.const 0)) (unreachable)) (else (unreachable)))
		(if (i32.const 0) (then (unreachable)) (else (br_table 0 (i32.const 0)) (unreachable)))
		(i32.const 19)
	)

	(func $break-value (export "break-value") (param i32) (result i32)
		(if (result i32) (local.get 0)
			(then (br 0 (i32.const 18)) (i32.const 19))
			(else (br 0 (i32.const 21)) (i32.const 20))
		)
	)

	;;   (func (export "break-multi-value") (param i32) (result i32 i32 i64)
	;;     (if (result i32 i32 i64) (local.get 0)
	;;       (then
	;;         (br 0 (i32.const 18) (i32.const -18) (i64.const 18))
	;;         (i32.const 19) (i32.const -19) (i64.const 19)
	;;       )
	;;       (else
	;;         (br 0 (i32.const -18) (i32.const 18) (i64.const -18))
	;;         (i32.const -19) (i32.const 19) (i64.const -19)
	;;       )
	;;     )
	;;   )

	;;   (func (export "param") (param i32) (result i32)
	;;     (i32.const 1)
	;;     (if (param i32) (result i32) (local.get 0)
	;;       (then (i32.const 2) (i32.add))
	;;       (else (i32.const -2) (i32.add))
	;;     )
	;;   )

	;;   (func (export "params") (param i32) (result i32)
	;;     (i32.const 1)
	;;     (i32.const 2)
	;;     (if (param i32 i32) (result i32) (local.get 0)
	;;       (then (i32.add))
	;;       (else (i32.sub))
	;;     )
	;;   )

	;;   (func (export "params-id") (param i32) (result i32)
	;;     (i32.const 1)
	;;     (i32.const 2)
	;;     (if (param i32 i32) (result i32 i32) (local.get 0) (then))
	;;     (i32.add)
	;;   )

	;;   (func (export "param-break") (param i32) (result i32)
	;;     (i32.const 1)
	;;     (if (param i32) (result i32) (local.get 0)
	;;       (then (i32.const 2) (i32.add) (br 0))
	;;       (else (i32.const -2) (i32.add) (br 0))
	;;     )
	;;   )

	;;   (func (export "params-break") (param i32) (result i32)
	;;     (i32.const 1)
	;;     (i32.const 2)
	;;     (if (param i32 i32) (result i32) (local.get 0)
	;;       (then (i32.add) (br 0))
	;;       (else (i32.sub) (br 0))
	;;     )
	;;   )

	;;   (func (export "params-id-break") (param i32) (result i32)
	;;     (i32.const 1)
	;;     (i32.const 2)
	;;     (if (param i32 i32) (result i32 i32) (local.get 0) (then (br 0)))
	;;     (i32.add)
	;;   )

	(func $effects (export "effects") (param i32) (result i32)
		(local i32)
		(if
			(block (result i32) (local.set 1 (i32.const 1)) (local.get 0))
			(then
				(local.set 1 (i32.mul (local.get 1) (i32.const 3)))
				(local.set 1 (i32.sub (local.get 1) (i32.const 5)))
				(local.set 1 (i32.mul (local.get 1) (i32.const 7)))
				(br 0)
				(local.set 1 (i32.mul (local.get 1) (i32.const 100)))
			)
			(else
				(local.set 1 (i32.mul (local.get 1) (i32.const 5)))
				(local.set 1 (i32.sub (local.get 1) (i32.const 7)))
				(local.set 1 (i32.mul (local.get 1) (i32.const 3)))
				(br 0)
				(local.set 1 (i32.mul (local.get 1) (i32.const 1000)))
			)
		)
		(local.get 1)
	)

	;; Examples

	;;   (func $add64_u_with_carry (export "add64_u_with_carry")
	;;     (param $i i64) (param $j i64) (param $c i32) (result i64 i32)
	;;     (local $k i64)
	;;     (local.set $k
	;;       (i64.add
	;;         (i64.add (local.get $i) (local.get $j))
	;;         (i64.extend_i32_u (local.get $c))
	;;       )
	;;     )
	;;     (return (local.get $k) (i64.lt_u (local.get $k) (local.get $i)))
	;;   )

	;;   (func $add64_u_saturated (export "add64_u_saturated")
	;;     (param i64 i64) (result i64)
	;;     (call $add64_u_with_carry (local.get 0) (local.get 1) (i32.const 0))
	;;     (if (param i64) (result i64)
	;;       (then (drop) (i64.const -1))
	;;     )
	;;   )

	;; Block signature syntax

	(type $block-sig-1 (func))
	(type $block-sig-2 (func (result i32)))
	(type $block-sig-3 (func (param $x i32)))
	;;   (type $block-sig-4 (func (param i32 f64 i32) (result i32 f64 i32)))

	;;   (func (export "type-use")
	;;     (if (type $block-sig-1) (i32.const 1) (then))
	;;     (if (type $block-sig-2) (i32.const 1)
	;;       (then (i32.const 0)) (else (i32.const 2))
	;;     )
	;;     (if (type $block-sig-3) (i32.const 1) (then (drop)) (else (drop)))
	;;     (i32.const 0) (f64.const 0) (i32.const 0)
	;;     (if (type $block-sig-4) (i32.const 1) (then))
	;;     (drop) (drop) (drop)
	;;     (if (type $block-sig-2) (result i32) (i32.const 1)
	;;       (then (i32.const 0)) (else (i32.const 2))
	;;     )
	;;     (if (type $block-sig-3) (param i32) (i32.const 1)
	;;       (then (drop)) (else (drop))
	;;     )
	;;     (i32.const 0) (f64.const 0) (i32.const 0)
	;;     (if (type $block-sig-4)
	;;       (param i32) (param f64 i32) (result i32 f64) (result i32)
	;;       (i32.const 1) (then)
	;;     )
	;;     (drop) (drop) (drop)
	;;   )

	(func $_start (export "_start")
		(call $empty (i32.const 0))
		(call $empty (i32.const 1))
		(call $empty (i32.const 100))
		(call $empty (i32.const -2))

		(if (i32.ne (call $singular (i32.const 0)) (i32.const 8)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $singular (i32.const 1)) (i32.const 7)) (then     
			(call $proc_exit (i32.const 2))
		))

		(if (i32.ne (call $singular (i32.const 10)) (i32.const 7)) (then     
			(call $proc_exit (i32.const 3))
		))

		(if (i32.ne (call $singular (i32.const -10)) (i32.const 7)) (then     
			(call $proc_exit (i32.const 4))
		))

		;; (if (i32.ne (call $multi (i32.const 0)) (i32.const 9) (i32.const -1)) (then     
		;; 	(call $proc_exit (i32.const 5))
		;; ))

		;; (if (i32.ne (call $multi (i32.const 1)) (i32.const 8) (i32.const 1)) (then     
		;; 	(call $proc_exit (i32.const 6))
		;; ))

		;; (if (i32.ne (call $multi (i32.const 13)) (i32.const 8) (i32.const 1)) (then     
		;; 	(call $proc_exit (i32.const 7))
		;; ))

		;; (if (i32.ne (call $multi (i32.const -5)) (i32.const 8) (i32.const 1)) (then     
		;; 	(call $proc_exit (i32.const 8))
		;; ))

		(if (i32.ne (call $nested (i32.const 0) (i32.const 0)) (i32.const 11)) (then     
			(call $proc_exit (i32.const 9))
		))

		(if (i32.ne (call $nested (i32.const 1) (i32.const 0)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 10))
		))

		(if (i32.ne (call $nested (i32.const 0) (i32.const 1)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 11))
		))

		(if (i32.ne (call $nested (i32.const 3) (i32.const 2)) (i32.const 9)) (then     
			(call $proc_exit (i32.const 12))
		))

		(if (i32.ne (call $nested (i32.const 0) (i32.const -100)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 13))
		))

		(if (i32.ne (call $nested (i32.const 10) (i32.const 10)) (i32.const 9)) (then     
			(call $proc_exit (i32.const 14))
		))

		(if (i32.ne (call $nested (i32.const 0) (i32.const -1)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 15))
		))

		(if (i32.ne (call $nested (i32.const -111) (i32.const -2)) (i32.const 9)) (then     
			(call $proc_exit (i32.const 16))
		))

		(if (i32.ne (call $as-select-first (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 17))
		))

		(if (i32.ne (call $as-select-first (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 18))
		))

		(if (i32.ne (call $as-select-mid (i32.const 0)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 19))
		))

		(if (i32.ne (call $as-select-mid (i32.const 1)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 20))
		))

		(if (i32.ne (call $as-select-last (i32.const 0)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 21))
		))

		(if (i32.ne (call $as-select-last (i32.const 1)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 22))
		))

		(if (i32.ne (call $as-loop-first (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 23))
		))

		(if (i32.ne (call $as-loop-first (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 24))
		))

		(if (i32.ne (call $as-loop-mid (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 25))
		))

		(if (i32.ne (call $as-loop-mid (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 26))
		))

		(if (i32.ne (call $as-loop-last (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 27))
		))

		(if (i32.ne (call $as-loop-last (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 28))
		))

		(if (i32.ne (call $as-if-condition (i32.const 0)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 29))
		))

		(if (i32.ne (call $as-if-condition (i32.const 1)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 30))
		))

		(if (i32.ne (call $as-br_if-first (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 31))
		))

		(if (i32.ne (call $as-br_if-first (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 32))
		))

		(if (i32.ne (call $as-br_if-last (i32.const 0)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 33))
		))

		(if (i32.ne (call $as-br_if-last (i32.const 1)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 34))
		))

		(if (i32.ne (call $as-br_table-first (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 35))
		))

		(if (i32.ne (call $as-br_table-first (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 36))
		))

		(if (i32.ne (call $as-br_table-last (i32.const 0)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 37))
		))

		(if (i32.ne (call $as-br_table-last (i32.const 1)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 38))
		))

		(if (i32.ne (call $as-call_indirect-first (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 39))
		))

		(if (i32.ne (call $as-call_indirect-first (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 40))
		))

		(if (i32.ne (call $as-call_indirect-mid (i32.const 0)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i32.ne (call $as-call_indirect-mid (i32.const 1)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 42))
		))

		(if (i32.ne (call $as-call_indirect-last (i32.const 0)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 43))
		))

		;; (assert_trap (invoke "as-call_indirect-last" (i32.const 1)) "undefined element")

		(call $as-store-first (i32.const 0))
		(call $as-store-first (i32.const 1))
		(call $as-store-last (i32.const 0))
		(call $as-store-last (i32.const 1))

		(if (i32.ne (call $as-memory.grow-value (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 44))
		))

		(if (i32.ne (call $as-memory.grow-value (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 45))
		))

		(if (i32.ne (call $as-call-value (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 46))
		))

		(if (i32.ne (call $as-call-value (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 47))
		))

		(if (i32.ne (call $as-return-value (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 48))
		))

		(if (i32.ne (call $as-return-value (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 49))
		))

		(call $as-drop-operand (i32.const 0))
		(call $as-drop-operand (i32.const 1))

		(if (i32.ne (call $as-br-value (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 50))
		))

		(if (i32.ne (call $as-br-value (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 51))
		))

		(if (i32.ne (call $as-local.set-value (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 52))
		))

		(if (i32.ne (call $as-local.set-value (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 53))
		))

		(if (i32.ne (call $as-local.tee-value (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 54))
		))

		(if (i32.ne (call $as-local.tee-value (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 55))
		))

		(if (i32.ne (call $as-global.set-value (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 56))
		))

		(if (i32.ne (call $as-global.set-value (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 57))
		))

		(if (i32.ne (call $as-load-operand (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 58))
		))

		(if (i32.ne (call $as-load-operand (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 59))
		))

		(if (i32.ne (call $as-binary-operand (i32.const 0) (i32.const 0)) (i32.const 15)) (then     
			(call $proc_exit (i32.const 63))
		))

		(if (i32.ne (call $as-binary-operand (i32.const 0) (i32.const 1)) (i32.const -12)) (then     
			(call $proc_exit (i32.const 64))
		))

		(if (i32.ne (call $as-binary-operand (i32.const 1) (i32.const 0)) (i32.const -15)) (then     
			(call $proc_exit (i32.const 65))
		))

		(if (i32.ne (call $as-binary-operand (i32.const 1) (i32.const 1)) (i32.const 12)) (then     
			(call $proc_exit (i32.const 66))
		))

		(if (i32.ne (call $as-test-operand (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 67))
		))

		(if (i32.ne (call $as-test-operand (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 68))
		))

		(if (i32.ne (call $as-compare-operand (i32.const 0) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 69))
		))

		(if (i32.ne (call $as-compare-operand (i32.const 0) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 70))
		))

		(if (i32.ne (call $as-compare-operand (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 71))
		))

		(if (i32.ne (call $as-compare-operand (i32.const 1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 72))
		))

		;; (if (i32.ne (call $as-binary-operands (i32.const 0)) (i32.const -12)) (then     
		;; 	(call $proc_exit (i32.const 73))
		;; ))

		;; (if (i32.ne (call $as-binary-operands (i32.const 1)) (i32.const 12)) (then     
		;; 	(call $proc_exit (i32.const 74))
		;; ))

		;; (if (i32.ne (call $as-compare-operands (i32.const 0)) (i32.const 1)) (then     
		;; 	(call $proc_exit (i32.const 75))
		;; ))

		;; (if (i32.ne (call $as-compare-operands (i32.const 1)) (i32.const 0)) (then     
		;; 	(call $proc_exit (i32.const 76))
		;; ))

		;; (if (i32.ne (call $as-mixed-operands (i32.const 0)) (i32.const -3)) (then     
		;; 	(call $proc_exit (i32.const 77))
		;; ))

		;; (if (i32.ne (call $as-mixed-operands (i32.const 1)) (i32.const 27)) (then     
		;; 	(call $proc_exit (i32.const 78))
		;; ))

		(if (i32.ne (call $break-bare) (i32.const 19)) (then     
			(call $proc_exit (i32.const 79))
		))

		(if (i32.ne (call $break-value (i32.const 1)) (i32.const 18)) (then     
			(call $proc_exit (i32.const 80))
		))

		(if (i32.ne (call $break-value (i32.const 0)) (i32.const 21)) (then     
			(call $proc_exit (i32.const 81))
		))

		;; (assert_return (invoke "break-multi-value" (i32.const 0))
		;;   (i32.const -18) (i32.const 18) (i64.const -18)
		;; )

		;; (assert_return (invoke "break-multi-value" (i32.const 1))
		;;   (i32.const 18) (i32.const -18) (i64.const 18)
		;; )

		;; (if (i32.ne (call $param (i32.const 0)) (i32.const -1)) (then     
		;; 	(call $proc_exit (i32.const 84))
		;; ))

		;; (if (i32.ne (call $param (i32.const 1)) (i32.const 3)) (then     
		;; 	(call $proc_exit (i32.const 85))
		;; ))

		;; (if (i32.ne (call $params (i32.const 0)) (i32.const -1)) (then     
		;; 	(call $proc_exit (i32.const 86))
		;; ))

		;; (if (i32.ne (call $params (i32.const 1)) (i32.const 3)) (then     
		;; 	(call $proc_exit (i32.const 87))
		;; ))

		;; (if (i32.ne (call $params-id (i32.const 0)) (i32.const 3)) (then     
		;; 	(call $proc_exit (i32.const 88))
		;; ))

		;; (if (i32.ne (call $params-id (i32.const 1)) (i32.const 3)) (then     
		;; 	(call $proc_exit (i32.const 89))
		;; ))

		;; (if (i32.ne (call $param-break (i32.const 0)) (i32.const -1)) (then     
		;; 	(call $proc_exit (i32.const 90))
		;; ))

		;; (if (i32.ne (call $param-break (i32.const 1)) (i32.const 3)) (then     
		;; 	(call $proc_exit (i32.const 91))
		;; ))

		;; (if (i32.ne (call $params-break (i32.const 0)) (i32.const -1)) (then     
		;; 	(call $proc_exit (i32.const 92))
		;; ))

		;; (if (i32.ne (call $params-break (i32.const 1)) (i32.const 3)) (then     
		;; 	(call $proc_exit (i32.const 93))
		;; ))

		;; (if (i32.ne (call $params-id-break (i32.const 0)) (i32.const 3)) (then     
		;; 	(call $proc_exit (i32.const 94))
		;; ))

		;; (if (i32.ne (call $params-id-break (i32.const 1)) (i32.const 3)) (then     
		;; 	(call $proc_exit (i32.const 95))
		;; ))
		
		(if (i32.ne (call $effects (i32.const 1)) (i32.const -14)) (then     
			(call $proc_exit (i32.const 96))
		))

		(if (i32.ne (call $effects (i32.const 0)) (i32.const -6)) (then     
			(call $proc_exit (i32.const 97))
		))

		;; (assert_return
		;;   (invoke "add64_u_with_carry" (i64.const 0) (i64.const 0) (i32.const 0))
		;;   (i64.const 0) (i32.const 0)
		;; )
		
		;; (assert_return
		;;   (invoke "add64_u_with_carry" (i64.const 100) (i64.const 124) (i32.const 0))
		;;   (i64.const 224) (i32.const 0)
		;; )
		
		;; (assert_return
		;;   (invoke "add64_u_with_carry" (i64.const -1) (i64.const 0) (i32.const 0))
		;;   (i64.const -1) (i32.const 0)
		;; )
		
		;; (assert_return
		;;   (invoke "add64_u_with_carry" (i64.const -1) (i64.const 1) (i32.const 0))
		;;   (i64.const 0) (i32.const 1)
		;; )
		
		;; (assert_return
		;;   (invoke "add64_u_with_carry" (i64.const -1) (i64.const -1) (i32.const 0))
		;;   (i64.const -2) (i32.const 1)
		;; )
		
		;; (assert_return
		;;   (invoke "add64_u_with_carry" (i64.const -1) (i64.const 0) (i32.const 1))
		;;   (i64.const 0) (i32.const 1)
		;; )
		
		;; (assert_return
		;;   (invoke "add64_u_with_carry" (i64.const -1) (i64.const 1) (i32.const 1))
		;;   (i64.const 1) (i32.const 1)
		;; )

		;; (assert_return
		;;   (invoke "add64_u_with_carry" (i64.const 0x8000000000000000) (i64.const 0x8000000000000000) (i32.const 0))
		;;   (i64.const 0) (i32.const 1)
		;; )

		;; (if (i32.ne (call $add64_u_saturated (i64.const 0) (i64.const 0)) (i64.const 0)) (then     
		;; 	(call $proc_exit (i32.const 106))
		;; ))

		;; (if (i32.ne (call $add64_u_saturated (i64.const 1230) (i64.const 23)) (i64.const 1253)) (then     
		;; 	(call $proc_exit (i32.const 107))
		;; ))

		;; (if (i32.ne (call $add64_u_saturated (i64.const -1) (i64.const 0)) (i64.const -1)) (then     
		;; 	(call $proc_exit (i32.const 108))
		;; ))

		;; (if (i32.ne (call $add64_u_saturated (i64.const -1) (i64.const 1)) (i64.const -1)) (then     
		;; 	(call $proc_exit (i32.const 109))
		;; ))

		;; (if (i32.ne (call $add64_u_saturated (i64.const -1) (i64.const -1)) (i64.const -1)) (then     
		;; 	(call $proc_exit (i32.const 110))
		;; ))

		;; (if (i32.ne (call $add64_u_saturated (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i64.const -1)) (then     
		;; 	(call $proc_exit (i32.const 111))
		;; ))

		;; (call $type-use)

		(call $proc_exit (i32.const 0))
	)
)
