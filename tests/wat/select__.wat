(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
  
	;; Auxiliary
	(func $dummy)
	(table $tab funcref (elem $dummy))
	(memory 1)

	(func $select-i32 (export "select-i32") (param i32 i32 i32) (result i32)
		(select (local.get 0) (local.get 1) (local.get 2))
	)
	(func $select-i64 (export "select-i64") (param i64 i64 i32) (result i64)
		(select (local.get 0) (local.get 1) (local.get 2))
	)
	(func $select-f32 (export "select-f32") (param f32 f32 i32) (result f32)
		(select (local.get 0) (local.get 1) (local.get 2))
	)
	(func $select-f64 (export "select-f64") (param f64 f64 i32) (result f64)
		(select (local.get 0) (local.get 1) (local.get 2))
	)

	;;   (func (export "select-i32-t") (param i32 i32 i32) (result i32)
	;;     (select (result i32) (local.get 0) (local.get 1) (local.get 2))
	;;   )

	;;   (func (export "select-i64-t") (param i64 i64 i32) (result i64)
	;;     (select (result i64) (local.get 0) (local.get 1) (local.get 2))
	;;   )
	;;   (func (export "select-f32-t") (param f32 f32 i32) (result f32)
	;;     (select (result f32) (local.get 0) (local.get 1) (local.get 2))
	;;   )
	;;   (func (export "select-f64-t") (param f64 f64 i32) (result f64)
	;;     (select (result f64) (local.get 0) (local.get 1) (local.get 2))
	;;   )
	;;   (func (export "select-funcref") (param funcref funcref i32) (result funcref)
	;;     (select (result funcref) (local.get 0) (local.get 1) (local.get 2))
	;;   )
	;;   (func (export "select-externref") (param externref externref i32) (result externref)
	;;     (select (result externref) (local.get 0) (local.get 1) (local.get 2))
	;;   )

	;; As the argument of control constructs and instructions

	(func $as-select-first (export "as-select-first") (param i32) (result i32)
		(select (select (i32.const 0) (i32.const 1) (local.get 0)) (i32.const 2) (i32.const 3))
	)

	(func $as-select-mid (export "as-select-mid") (param i32) (result i32)
		(select (i32.const 2) (select (i32.const 0) (i32.const 1) (local.get 0)) (i32.const 3))
	)
	
	(func $as-select-last (export "as-select-last") (param i32) (result i32)
		(select (i32.const 2) (i32.const 3) (select (i32.const 0) (i32.const 1) (local.get 0)))
	)

	(func $as-loop-first (export "as-loop-first") (param i32) (result i32)
		(loop (result i32) (select (i32.const 2) (i32.const 3) (local.get 0)) (call $dummy) (call $dummy))
	)
	
	(func $as-loop-mid (export "as-loop-mid") (param i32) (result i32)
		(loop (result i32) (call $dummy) (select (i32.const 2) (i32.const 3) (local.get 0)) (call $dummy))
	)

	(func $as-loop-last (export "as-loop-last") (param i32) (result i32)
		(loop (result i32) (call $dummy) (call $dummy) (select (i32.const 2) (i32.const 3) (local.get 0)))
	)

	(func $as-if-condition (export "as-if-condition") (param i32)
		(select (i32.const 2) (i32.const 3) (local.get 0)) (if (then (call $dummy)))
	)

	(func $as-if-then (export "as-if-then") (param i32) (result i32)
		(if (result i32) (i32.const 1) (then (select (i32.const 2) (i32.const 3) (local.get 0))) (else (i32.const 4)))
	)

	(func $as-if-else (export "as-if-else") (param i32) (result i32)
		(if (result i32) (i32.const 0) (then (i32.const 2)) (else (select (i32.const 2) (i32.const 3) (local.get 0))))
	)

	(func $as-br_if-first (export "as-br_if-first") (param i32) (result i32)
		(block (result i32) (br_if 0 (select (i32.const 2) (i32.const 3) (local.get 0)) (i32.const 4)))
	)
	(func $as-br_if-last (export "as-br_if-last") (param i32) (result i32)
		(block (result i32) (br_if 0 (i32.const 2) (select (i32.const 2) (i32.const 3) (local.get 0))))
	)

	(func $as-br_table-first (export "as-br_table-first") (param i32) (result i32)
		(block (result i32) (select (i32.const 2) (i32.const 3) (local.get 0)) (i32.const 2) (br_table 0 0))
	)
	(func $as-br_table-last (export "as-br_table-last") (param i32) (result i32)
		(block (result i32) (i32.const 2) (select (i32.const 2) (i32.const 3) (local.get 0)) (br_table 0 0))
	)

	(func $func (param i32 i32) (result i32) (local.get 0))
	(type $check (func (param i32 i32) (result i32)))
	;;   (table $t funcref (elem $func))
	;;   (func (export "as-call_indirect-first") (param i32) (result i32)
	;;     (block (result i32)
	;;       (call_indirect $t (type $check)
	;;         (select (i32.const 2) (i32.const 3) (local.get 0)) (i32.const 1) (i32.const 0)
	;;       )
	;;     )
	;;   )
	;;   (func (export "as-call_indirect-mid") (param i32) (result i32)
	;;     (block (result i32)
	;;       (call_indirect $t (type $check)
	;;         (i32.const 1) (select (i32.const 2) (i32.const 3) (local.get 0)) (i32.const 0)
	;;       )
	;;     )
	;;   )
	;;   (func (export "as-call_indirect-last") (param i32) (result i32)
	;;     (block (result i32)
	;;       (call_indirect $t (type $check)
	;;         (i32.const 1) (i32.const 4) (select (i32.const 2) (i32.const 3) (local.get 0))
	;;       )
	;;     )
	;;   )

	(func $as-store-first (export "as-store-first") (param i32)
		(select (i32.const 0) (i32.const 4) (local.get 0)) (i32.const 1) (i32.store)
	)
	(func $as-store-last (export "as-store-last") (param i32)
		(i32.const 8) (select (i32.const 1) (i32.const 2) (local.get 0)) (i32.store)
	)

	(func $as-memory.grow-value (export "as-memory.grow-value") (param i32) (result i32)
		(memory.grow (select (i32.const 1) (i32.const 2) (local.get 0)))
	)

	(func $f (param i32) (result i32) (local.get 0))

	(func $as-call-value (export "as-call-value") (param i32) (result i32)
		(call $f (select (i32.const 1) (i32.const 2) (local.get 0)))
	)
	(func $as-return-value (export "as-return-value") (param i32) (result i32)
		(select (i32.const 1) (i32.const 2) (local.get 0)) (return)
	)
	(func $as-drop-operand (export "as-drop-operand") (param i32)
		(drop (select (i32.const 1) (i32.const 2) (local.get 0)))
	)
	(func $as-br-value (export "as-br-value") (param i32) (result i32)
		(block (result i32) (br 0 (select (i32.const 1) (i32.const 2) (local.get 0))))
	)
	(func $as-local.set-value (export "as-local.set-value") (param i32) (result i32)
		(local i32) (local.set 0 (select (i32.const 1) (i32.const 2) (local.get 0))) (local.get 0)
	)
	(func $as-local.tee-value (export "as-local.tee-value") (param i32) (result i32)
		(local.tee 0 (select (i32.const 1) (i32.const 2) (local.get 0)))
	)
	(global $a (mut i32) (i32.const 10))
	(func $as-global.set-value (export "as-global.set-value") (param i32) (result i32)
		(global.set $a (select (i32.const 1) (i32.const 2) (local.get 0)))
		(global.get $a)
	)
	(func $as-load-operand (export "as-load-operand") (param i32) (result i32)
		(i32.load (select (i32.const 0) (i32.const 4) (local.get 0)))
	)

	(func $as-unary-operand (export "as-unary-operand") (param i32) (result i32)
		(i32.eqz (select (i32.const 0) (i32.const 1) (local.get 0)))
	)
	(func $as-binary-operand (export "as-binary-operand") (param i32) (result i32)
		(i32.mul
		(select (i32.const 1) (i32.const 2) (local.get 0))
		(select (i32.const 1) (i32.const 2) (local.get 0))
		)
	)
	(func $as-test-operand (export "as-test-operand") (param i32) (result i32)
		(block (result i32)
		(i32.eqz (select (i32.const 0) (i32.const 1) (local.get 0)))
		)
	)

	(func $as-compare-left (export "as-compare-left") (param i32) (result i32)
		(block (result i32)
		(i32.le_s (select (i32.const 1) (i32.const 2) (local.get 0)) (i32.const 1))
		)
	)
	(func $as-compare-right (export "as-compare-right") (param i32) (result i32)
		(block (result i32)
		(i32.ne (i32.const 1) (select (i32.const 0) (i32.const 1) (local.get 0)))
		)
	)

	(func $as-convert-operand (export "as-convert-operand") (param i32) (result i32)
		(block (result i32)
			(i32.wrap_i64 (select (i64.const 1) (i64.const 0) (local.get 0)))
		)
	)

	(func $_start (export "_start")
		(if (i32.ne (call $select-i32 (i32.const 1) (i32.const 2) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $select-i64 (i64.const 2) (i64.const 1) (i32.const 1)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $select-f32 (f32.const 1) (f32.const 2) (i32.const 1)) (f32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (f64.ne (call $select-f64 (f64.const 1) (f64.const 2) (i32.const 1)) (f64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $select-i32 (i32.const 1) (i32.const 2) (i32.const 0)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $select-i32 (i32.const 2) (i32.const 1) (i32.const 0)) (i32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $select-i64 (i64.const 2) (i64.const 1) (i32.const -1)) (i64.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $select-i64 (i64.const 2) (i64.const 1) (i32.const 0xf0f0f0f0)) (i64.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $select-f32 (f32.const nan) (f32.const 1) (i32.const 1)) (f32.const nan))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $select-f32 (f32.const nan:0x20304) (f32.const 1) (i32.const 1)) (f32.const nan:0x20304))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $select-f32 (f32.const nan) (f32.const 1) (i32.const 0)) (f32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $select-f32 (f32.const nan:0x20304) (f32.const 1) (i32.const 0)) (f32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $select-f32 (f32.const 2) (f32.const nan) (i32.const 1)) (f32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $select-f32 (f32.const 2) (f32.const nan:0x20304) (i32.const 1)) (f32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $select-f32 (f32.const 2) (f32.const nan) (i32.const 0)) (f32.const nan))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $select-f32 (f32.const 2) (f32.const nan:0x20304) (i32.const 0)) (f32.const nan:0x20304))(then     
			(call $proc_exit (i32.const 1))
		))


		(if (f64.ne (call $select-f64 (f64.const nan) (f64.const 1) (i32.const 1)) (f64.const nan))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f64.ne (call $select-f64 (f64.const nan:0x20304) (f64.const 1) (i32.const 1)) (f64.const nan:0x20304))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f64.ne (call $select-f64 (f64.const nan) (f64.const 1) (i32.const 0)) (f64.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f64.ne (call $select-f64 (f64.const nan:0x20304) (f64.const 1) (i32.const 0)) (f64.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f64.ne (call $select-f64 (f64.const 2) (f64.const nan) (i32.const 1)) (f64.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f64.ne (call $select-f64 (f64.const 2) (f64.const nan:0x20304) (i32.const 1)) (f64.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f64.ne (call $select-f64 (f64.const 2) (f64.const nan) (i32.const 0)) (f64.const nan))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (f64.ne (call $select-f64 (f64.const 2) (f64.const nan:0x20304) (i32.const 0)) (f64.const nan:0x20304))(then     
			(call $proc_exit (i32.const 1))
		))

		(call $proc_exit (i32.const 0))

		;; Requires result in select
		;; (assert_return (invoke "select-i32-t" (i32.const 1) (i32.const 2) (i32.const 1)) (i32.const 1))
		;; (assert_return (invoke "select-i64-t" (i64.const 2) (i64.const 1) (i32.const 1)) (i64.const 2))
		;; (assert_return (invoke "select-f32-t" (f32.const 1) (f32.const 2) (i32.const 1)) (f32.const 1))
		;; (assert_return (invoke "select-f64-t" (f64.const 1) (f64.const 2) (i32.const 1)) (f64.const 1))
		;; (assert_return (invoke "select-funcref" (ref.null func) (ref.null func) (i32.const 1)) (ref.null func))
		;; (assert_return (invoke "select-externref" (ref.extern 1) (ref.extern 2) (i32.const 1)) (ref.extern 1))
		;; (assert_return (invoke "select-i32-t" (i32.const 1) (i32.const 2) (i32.const 0)) (i32.const 2))
		;; (assert_return (invoke "select-i32-t" (i32.const 2) (i32.const 1) (i32.const 0)) (i32.const 1))
		;; (assert_return (invoke "select-i64-t" (i64.const 2) (i64.const 1) (i32.const -1)) (i64.const 2))
		;; (assert_return (invoke "select-i64-t" (i64.const 2) (i64.const 1) (i32.const 0xf0f0f0f0)) (i64.const 2))
		;; (assert_return (invoke "select-externref" (ref.extern 1) (ref.extern 2) (i32.const 0)) (ref.extern 2))
		;; (assert_return (invoke "select-externref" (ref.extern 2) (ref.extern 1) (i32.const 0)) (ref.extern 1))
		;; (assert_return (invoke "select-f32-t" (f32.const nan) (f32.const 1) (i32.const 1)) (f32.const nan))
		;; (assert_return (invoke "select-f32-t" (f32.const nan:0x20304) (f32.const 1) (i32.const 1)) (f32.const nan:0x20304))
		;; (assert_return (invoke "select-f32-t" (f32.const nan) (f32.const 1) (i32.const 0)) (f32.const 1))
		;; (assert_return (invoke "select-f32-t" (f32.const nan:0x20304) (f32.const 1) (i32.const 0)) (f32.const 1))
		;; (assert_return (invoke "select-f32-t" (f32.const 2) (f32.const nan) (i32.const 1)) (f32.const 2))
		;; (assert_return (invoke "select-f32-t" (f32.const 2) (f32.const nan:0x20304) (i32.const 1)) (f32.const 2))
		;; (assert_return (invoke "select-f32-t" (f32.const 2) (f32.const nan) (i32.const 0)) (f32.const nan))
		;; (assert_return (invoke "select-f32-t" (f32.const 2) (f32.const nan:0x20304) (i32.const 0)) (f32.const nan:0x20304))
		;; (assert_return (invoke "select-f64-t" (f64.const nan) (f64.const 1) (i32.const 1)) (f64.const nan))
		;; (assert_return (invoke "select-f64-t" (f64.const nan:0x20304) (f64.const 1) (i32.const 1)) (f64.const nan:0x20304))
		;; (assert_return (invoke "select-f64-t" (f64.const nan) (f64.const 1) (i32.const 0)) (f64.const 1))
		;; (assert_return (invoke "select-f64-t" (f64.const nan:0x20304) (f64.const 1) (i32.const 0)) (f64.const 1))
		;; (assert_return (invoke "select-f64-t" (f64.const 2) (f64.const nan) (i32.const 1)) (f64.const 2))
		;; (assert_return (invoke "select-f64-t" (f64.const 2) (f64.const nan:0x20304) (i32.const 1)) (f64.const 2))
		;; (assert_return (invoke "select-f64-t" (f64.const 2) (f64.const nan) (i32.const 0)) (f64.const nan))
		;; (assert_return (invoke "select-f64-t" (f64.const 2) (f64.const nan:0x20304) (i32.const 0)) (f64.const nan:0x20304))

		(if (i32.ne (call $as-select-first (i32.const 0)) (i32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-select-first (i32.const 1)) (i32.const 0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-select-mid (i32.const 0)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-select-mid (i32.const 1)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-select-last (i32.const 0)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-select-last (i32.const 1)) (i32.const 3))(then     
			(call $proc_exit (i32.const 1))
		))


		(if (i32.ne (call $as-loop-first (i32.const 0)) (i32.const 3))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-loop-first (i32.const 1)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-loop-mid (i32.const 0)) (i32.const 3))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-loop-mid (i32.const 1)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-loop-last (i32.const 0)) (i32.const 3))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-loop-last (i32.const 1)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(call $as-if-condition (i32.const 0))
		(call $as-if-condition (i32.const 1))


		(if (i32.ne (call $as-if-then (i32.const 0)) (i32.const 3))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-if-then (i32.const 1)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-if-else (i32.const 0)) (i32.const 3))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-if-else (i32.const 1)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))


		(if (i32.ne (call $as-br_if-first (i32.const 0)) (i32.const 3))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-br_if-first (i32.const 1)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-br_if-last (i32.const 0)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-br_if-last (i32.const 1)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))


		(if (i32.ne (call $as-br_table-first (i32.const 0)) (i32.const 3))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-br_table-first (i32.const 1)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-br_table-last (i32.const 0)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-br_table-last (i32.const 1)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		;; Requires select result
		;; (assert_return (invoke "as-call_indirect-first" (i32.const 0)) (i32.const 3))
		;; (assert_return (invoke "as-call_indirect-first" (i32.const 1)) (i32.const 2))
		;; (assert_return (invoke "as-call_indirect-mid" (i32.const 0)) (i32.const 1))
		;; (assert_return (invoke "as-call_indirect-mid" (i32.const 1)) (i32.const 1))
		;; (assert_trap (invoke "as-call_indirect-last" (i32.const 0)) "undefined element")
		;; (assert_trap (invoke "as-call_indirect-last" (i32.const 1)) "undefined element")

		(call $as-store-first (i32.const 0))
		(call $as-store-first (i32.const 1))
		(call $as-store-last (i32.const 0))
		(call $as-store-last (i32.const 1))

		(if (i32.ne (call $as-memory.grow-value (i32.const 0)) (i32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-memory.grow-value (i32.const 1)) (i32.const 3))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-call-value (i32.const 0)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-call-value (i32.const 1)) (i32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-return-value (i32.const 0)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-return-value (i32.const 1)) (i32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))


		(call $as-drop-operand (i32.const 0))
		(call $as-drop-operand (i32.const 1))

		(if (i32.ne (call $as-br-value (i32.const 0)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-br-value (i32.const 1)) (i32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-local.set-value (i32.const 0)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-local.set-value (i32.const 1)) (i32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-local.tee-value (i32.const 0)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-local.tee-value (i32.const 1)) (i32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-global.set-value (i32.const 0)) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-global.set-value (i32.const 1)) (i32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-load-operand (i32.const 0)) (i32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-load-operand (i32.const 1)) (i32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-unary-operand (i32.const 0)) (i32.const 0))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-unary-operand (i32.const 1)) (i32.const 1))(then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-binary-operand (i32.const 0)) (i32.const 4)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-binary-operand (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-test-operand (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-test-operand (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-compare-left (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-compare-left (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-compare-right (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-compare-right (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-convert-operand (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $as-convert-operand (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(call $proc_exit (i32.const 0))
	)
)
