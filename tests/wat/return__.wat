;; Test `return` operator

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))

	;; Auxiliary definition
	(func $dummy)

	(func $type-i32 (export "type-i32") (drop (i32.ctz (return))))
	(func $type-i64 (export "type-i64") (drop (i64.ctz (return))))
	(func $type-f32 (export "type-f32") (drop (f32.neg (return))))
	(func $type-f64 (export "type-f64") (drop (f64.neg (return))))

	(func $type-i32-value (export "type-i32-value") (result i32)
		(block (result i32) (i32.ctz (return (i32.const 1))))
	)
	(func $type-i64-value (export "type-i64-value") (result i64)
		(block (result i64) (i64.ctz (return (i64.const 2))))
	)
	(func $type-f32-value (export "type-f32-value") (result f32)
		(block (result f32) (f32.neg (return (f32.const 3))))
	)
	(func $type-f64-value (export "type-f64-value") (result f64)
		(block (result f64) (f64.neg (return (f64.const 4))))
	)

	(func $nullary (export "nullary") (return))
	(func $unary (export "unary") (result f64) (return (f64.const 3)))

	(func $as-func-first (export "as-func-first") (result i32)
		(return (i32.const 1)) (i32.const 2)
	)
	(func $as-func-mid (export "as-func-mid") (result i32)
		(call $dummy) (return (i32.const 2)) (i32.const 3)
	)
	(func $as-func-last (export "as-func-last")
		(nop) (call $dummy) (return)
	)
	(func $as-func-value (export "as-func-value") (result i32)
		(nop) (call $dummy) (return (i32.const 3))
	)

	(func $as-block-first (export "as-block-first")
		(block (return) (call $dummy))
	)
	(func $as-block-mid (export "as-block-mid")
		(block (call $dummy) (return) (call $dummy))
	)
	(func $as-block-last (export "as-block-last")
		(block (nop) (call $dummy) (return))
	)
	(func $as-block-value (export "as-block-value") (result i32)
		(block (result i32) (nop) (call $dummy) (return (i32.const 2)))
	)

	(func $as-loop-first (export "as-loop-first") (result i32)
		(loop (result i32) (return (i32.const 3)) (i32.const 2))
	)
	(func $as-loop-mid (export "as-loop-mid") (result i32)
		(loop (result i32) (call $dummy) (return (i32.const 4)) (i32.const 2))
	)
	(func $as-loop-last (export "as-loop-last") (result i32)
		(loop (result i32) (nop) (call $dummy) (return (i32.const 5)))
	)

	(func $as-br-value (export "as-br-value") (result i32)
		(block (result i32) (br 0 (return (i32.const 9))))
	)

	(func $as-br_if-cond (export "as-br_if-cond")
		(block (br_if 0 (return)))
	)
	(func $as-br_if-value (export "as-br_if-value") (result i32)
		(block (result i32)
		(drop (br_if 0 (return (i32.const 8)) (i32.const 1))) (i32.const 7)
		)
	)
	(func $as-br_if-value-cond (export "as-br_if-value-cond") (result i32)
		(block (result i32)
		(drop (br_if 0 (i32.const 6) (return (i32.const 9)))) (i32.const 7)
		)
	)

	(func $as-br_table-index (export "as-br_table-index") (result i64)
		(block (br_table 0 0 0 (return (i64.const 9)))) (i64.const -1)
	)
	(func $as-br_table-value (export "as-br_table-value") (result i32)
		(block (result i32)
		(br_table 0 0 0 (return (i32.const 10)) (i32.const 1)) (i32.const 7)
		)
	)
	(func $as-br_table-value-index (export "as-br_table-value-index") (result i32)
		(block (result i32)
		(br_table 0 0 (i32.const 6) (return (i32.const 11))) (i32.const 7)
		)
	)

	(func $as-return-value (export "as-return-value") (result i64)
		(return (return (i64.const 7)))
	)

	(func $as-if-cond (export "as-if-cond") (result i32)
		(if (result i32)
		(return (i32.const 2)) (then (i32.const 0)) (else (i32.const 1))
		)
	)
	(func $as-if-then (export "as-if-then") (param i32 i32) (result i32)
		(if (result i32)
		(local.get 0) (then (return (i32.const 3))) (else (local.get 1))
		)
	)
	(func $as-if-else (export "as-if-else") (param i32 i32) (result i32)
		(if (result i32)
		(local.get 0) (then (local.get 1)) (else (return (i32.const 4)))
		)
	)

	(func $as-select-first (export "as-select-first") (param i32 i32) (result i32)
		(select (return (i32.const 5)) (local.get 0) (local.get 1))
	)
	(func $as-select-second (export "as-select-second") (param i32 i32) (result i32)
		(select (local.get 0) (return (i32.const 6)) (local.get 1))
	)
	(func $as-select-cond (export "as-select-cond") (result i32)
		(select (i32.const 0) (i32.const 1) (return (i32.const 7)))
	)

	(func $f (param i32 i32 i32) (result i32) (i32.const -1))
	(func $as-call-first (export "as-call-first") (result i32)
		(call $f (return (i32.const 12)) (i32.const 2) (i32.const 3))
	)
	(func $as-call-mid (export "as-call-mid") (result i32)
		(call $f (i32.const 1) (return (i32.const 13)) (i32.const 3))
	)
	(func $as-call-last (export "as-call-last") (result i32)
		(call $f (i32.const 1) (i32.const 2) (return (i32.const 14)))
	)

	(type $sig (func (param i32 i32 i32) (result i32)))
	(table funcref (elem $f))
	(func $as-call_indirect-func (export "as-call_indirect-func") (result i32)
		(call_indirect (type $sig)
		(return (i32.const 20)) (i32.const 1) (i32.const 2) (i32.const 3)
		)
	)
	(func $as-call_indirect-first (export "as-call_indirect-first") (result i32)
		(call_indirect (type $sig)
		(i32.const 0) (return (i32.const 21)) (i32.const 2) (i32.const 3)
		)
	)
	(func $as-call_indirect-mid (export "as-call_indirect-mid") (result i32)
		(call_indirect (type $sig)
		(i32.const 0) (i32.const 1) (return (i32.const 22)) (i32.const 3)
		)
	)
	(func $as-call_indirect-last (export "as-call_indirect-last") (result i32)
		(call_indirect (type $sig)
		(i32.const 0) (i32.const 1) (i32.const 2) (return (i32.const 23))
		)
	)

	(func $as-local.set-value (export "as-local.set-value") (result i32) (local f32)
		(local.set 0 (return (i32.const 17))) (i32.const -1)
	)
	(func $as-local.tee-value (export "as-local.tee-value") (result i32) (local i32)
		(local.tee 0 (return (i32.const 1)))
	)
	(global $a (mut i32) (i32.const 0))
	(func $as-global.set-value (export "as-global.set-value") (result i32)
		(global.set $a (return (i32.const 1)))
	)

	(memory 1)
	(export "memory" (memory 0))

	(func $as-load-address (export "as-load-address") (result f32)
		(f32.load (return (f32.const 1.7)))
	)
	(func $as-loadN-address (export "as-loadN-address") (result i64)
		(i64.load8_s (return (i64.const 30)))
	)

	(func $as-store-address (export "as-store-address") (result i32)
		(f64.store (return (i32.const 30)) (f64.const 7)) (i32.const -1)
	)
	(func $as-store-value (export "as-store-value") (result i32)
		(i64.store (i32.const 2) (return (i32.const 31))) (i32.const -1)
	)

	(func $as-storeN-address (export "as-storeN-address") (result i32)
		(i32.store8 (return (i32.const 32)) (i32.const 7)) (i32.const -1)
	)
	(func $as-storeN-value (export "as-storeN-value") (result i32)
		(i64.store16 (i32.const 2) (return (i32.const 33))) (i32.const -1)
	)

	(func $as-unary-operand (export "as-unary-operand") (result f32)
		(f32.neg (return (f32.const 3.4)))
	)

	(func $as-binary-left (export "as-binary-left") (result i32)
		(i32.add (return (i32.const 3)) (i32.const 10))
	)
	(func $as-binary-right (export "as-binary-right") (result i64)
		(i64.sub (i64.const 10) (return (i64.const 45)))
	)

	(func $as-test-operand (export "as-test-operand") (result i32)
		(i32.eqz (return (i32.const 44)))
	)

	(func $as-compare-left (export "as-compare-left") (result i32)
		(f64.le (return (i32.const 43)) (f64.const 10))
	)
	(func $as-compare-right (export "as-compare-right") (result i32)
		(f32.ne (f32.const 10) (return (i32.const 42)))
	)

	(func $as-convert-operand (export "as-convert-operand") (result i32)
		(i32.wrap_i64 (return (i32.const 41)))
	)

	(func $as-memory.grow-size (export "as-memory.grow-size") (result i32)
		(memory.grow (return (i32.const 40)))
	)

	(func $_start (export "_start")
		;; (if (i32.ne (call $select-i32 (i32.const 1) (i32.const 2) (i32.const 1)) (i32.const 1)) (then     
		;; 	(call $proc_exit (i32.const 1))
		;; ))

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

		(call $proc_exit (i32.const 0))

		(call $nullary)

		(if (f64.ne (call $unary) (f64.const 3)) (then     
			(call $proc_exit (i32.const 5))
		))

		(if (i32.ne (call $as-func-first) (i32.const 1)) (then     
			(call $proc_exit (i32.const 6))
		))

		(if (i32.ne (call $as-func-mid) (i32.const 2)) (then     
			(call $proc_exit (i32.const 7))
		))

		(call $as-func-last)
		
		(if (i32.ne (call $as-func-value) (i32.const 3)) (then     
			(call $proc_exit (i32.const 8))
		))

		(call $as-block-first)
		(call $as-block-mid)
		(call $as-block-last)

		(if (i32.ne (call $as-block-value) (i32.const 2)) (then     
			(call $proc_exit (i32.const 9))
		))

		(if (i32.ne (call $as-loop-first) (i32.const 3)) (then     
			(call $proc_exit (i32.const 10))
		))

		(if (i32.ne (call $as-loop-mid) (i32.const 4)) (then     
			(call $proc_exit (i32.const 11))
		))

		(if (i32.ne (call $as-loop-last) (i32.const 5)) (then     
			(call $proc_exit (i32.const 12))
		))

		(if (i32.ne (call $as-br-value) (i32.const 9)) (then     
			(call $proc_exit (i32.const 13))
		))

		(call $as-br_if-cond)

		(if (i32.ne (call $as-br_if-value) (i32.const 8)) (then     
			(call $proc_exit (i32.const 14))
		))

		(if (i32.ne (call $as-br_if-value-cond) (i32.const 9)) (then     
			(call $proc_exit (i32.const 15))
		))

		(if (i64.ne (call $as-br_table-index) (i64.const 9)) (then     
			(call $proc_exit (i32.const 16))
		))

		(if (i32.ne (call $as-br_table-value) (i32.const 10)) (then     
			(call $proc_exit (i32.const 17))
		))

		(if (i32.ne (call $as-br_table-value-index) (i32.const 11)) (then     
			(call $proc_exit (i32.const 18))
		))

		(if (i64.ne (call $as-return-value) (i64.const 7)) (then     
			(call $proc_exit (i32.const 19))
		))

		(if (i32.ne (call $as-if-cond) (i32.const 2)) (then     
			(call $proc_exit (i32.const 20))
		))

		(if (i32.ne (call $as-if-then (i32.const 1) (i32.const 6)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 21))
		))

		(if (i32.ne (call $as-if-then (i32.const 0) (i32.const 6)) (i32.const 6)) (then     
			(call $proc_exit (i32.const 22))
		))

		(if (i32.ne (call $as-if-else (i32.const 0) (i32.const 6)) (i32.const 4)) (then     
			(call $proc_exit (i32.const 23))
		))

		(if (i32.ne (call $as-if-else (i32.const 1) (i32.const 6)) (i32.const 6)) (then     
			(call $proc_exit (i32.const 24))
		))

		(if (i32.ne (call $as-select-first (i32.const 0) (i32.const 6)) (i32.const 5)) (then     
			(call $proc_exit (i32.const 25))
		))

		(if (i32.ne (call $as-select-first (i32.const 1) (i32.const 6)) (i32.const 5)) (then     
			(call $proc_exit (i32.const 26))
		))

		(if (i32.ne (call $as-select-second (i32.const 0) (i32.const 6)) (i32.const 6)) (then     
			(call $proc_exit (i32.const 27))
		))

		(if (i32.ne (call $as-select-second (i32.const 1) (i32.const 6)) (i32.const 6)) (then     
			(call $proc_exit (i32.const 28))
		))

		(if (i32.ne (call $as-select-cond) (i32.const 7)) (then     
			(call $proc_exit (i32.const 29))
		))

		(if (i32.ne (call $as-call-first) (i32.const 12)) (then     
			(call $proc_exit (i32.const 30))
		))

		(if (i32.ne (call $as-call-mid) (i32.const 13)) (then     
			(call $proc_exit (i32.const 31))
		))

		(if (i32.ne (call $as-call-last) (i32.const 14)) (then     
			(call $proc_exit (i32.const 32))
		))

		(if (i32.ne (call $as-call_indirect-func) (i32.const 20)) (then     
			(call $proc_exit (i32.const 33))
		))

		(if (i32.ne (call $as-call_indirect-first) (i32.const 21)) (then     
			(call $proc_exit (i32.const 34))
		))

		(if (i32.ne (call $as-call_indirect-mid) (i32.const 22)) (then     
			(call $proc_exit (i32.const 35))
		))

		(if (i32.ne (call $as-call_indirect-last) (i32.const 23)) (then     
			(call $proc_exit (i32.const 36))
		))

		(if (i32.ne (call $as-local.set-value) (i32.const 17)) (then     
			(call $proc_exit (i32.const 37))
		))

		(if (i32.ne (call $as-local.tee-value) (i32.const 1)) (then     
			(call $proc_exit (i32.const 38))
		))

		(if (i32.ne (call $as-global.set-value) (i32.const 1)) (then     
			(call $proc_exit (i32.const 39))
		))

		(if (f32.ne (call $as-load-address) (f32.const 1.7)) (then     
			(call $proc_exit (i32.const 40))
		))

		(if (i64.ne (call $as-loadN-address) (i64.const 30)) (then     
			(call $proc_exit (i32.const 41))
		))


		(if (i32.ne (call $as-store-address) (i32.const 30)) (then     
			(call $proc_exit (i32.const 42))
		))

		(if (i32.ne (call $as-store-value) (i32.const 31)) (then     
			(call $proc_exit (i32.const 43))
		))

		(if (i32.ne (call $as-storeN-address) (i32.const 32)) (then     
			(call $proc_exit (i32.const 44))
		))

		(if (i32.ne (call $as-storeN-value) (i32.const 33)) (then     
			(call $proc_exit (i32.const 45))
		))

		(if (f32.ne (call $as-unary-operand) (f32.const 3.4)) (then     
			(call $proc_exit (i32.const 46))
		))

		(if (i32.ne (call $as-binary-left) (i32.const 3)) (then     
			(call $proc_exit (i32.const 47))
		))

		(if (i64.ne (call $as-binary-right) (i64.const 45)) (then     
			(call $proc_exit (i32.const 48))
		))

		(if (i32.ne (call $as-test-operand) (i32.const 44)) (then     
			(call $proc_exit (i32.const 49))
		))

		(if (i32.ne (call $as-compare-left) (i32.const 43)) (then     
			(call $proc_exit (i32.const 50))
		))

		(if (i32.ne (call $as-compare-right) (i32.const 42)) (then     
			(call $proc_exit (i32.const 51))
		))

		(if (i32.ne (call $as-convert-operand) (i32.const 41)) (then     
			(call $proc_exit (i32.const 52))
		))

		(if (i32.ne (call $as-memory.grow-size) (i32.const 40)) (then     
			(call $proc_exit (i32.const 53))
		))
	)
)
