;; Test `call` operator

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))

	;; Auxiliary definitions
	(func $const-i32 (result i32) (i32.const 0x132))
	(func $const-i64 (result i64) (i64.const 0x164))
	(func $const-f32 (result f32) (f32.const 0xf32))
	(func $const-f64 (result f64) (f64.const 0xf64))
	
	;; error: multiple result values not currently supported.
	;; (func $const-i32-i64 (result i32 i64) (i32.const 0x132) (i64.const 0x164))

	(func $id-i32 (param i32) (result i32) (local.get 0))
	(func $id-i64 (param i64) (result i64) (local.get 0))
	(func $id-f32 (param f32) (result f32) (local.get 0))
	(func $id-f64 (param f64) (result f64) (local.get 0))
	
	;; error: multiple result values not currently supported.
	;; (func $id-i32-f64 (param i32 f64) (result i32 f64)
	;; 	(local.get 0) (local.get 1)
  	;; )

	;; error: multiple result values not currently supported.
	;; (func $swap-i32-i32 (param i32 i32) (result i32 i32)
	;; 	(local.get 1) (local.get 0)
	;; )

	;; error: multiple result values not currently supported.
	;; (func $swap-f32-f64 (param f32 f64) (result f64 f32)
	;; 	(local.get 1) (local.get 0)
	;; )

	;; error: multiple result values not currently supported.
	;; (func $swap-f64-i32 (param f64 i32) (result i32 f64)
	;; 	(local.get 1) (local.get 0)
	;; )

	(func $f32-i32 (param f32 i32) (result i32) (local.get 1))
	(func $i32-i64 (param i32 i64) (result i64) (local.get 1))
	(func $f64-f32 (param f64 f32) (result f32) (local.get 1))
	(func $i64-f64 (param i64 f64) (result f64) (local.get 1))

	;; Typing

	(func $type-i32 (export "type-i32") (result i32) (call $const-i32))
	(func $type-i64 (export "type-i64") (result i64) (call $const-i64))
	(func $type-f32 (export "type-f32") (result f32) (call $const-f32))
	(func $type-f64 (export "type-f64") (result f64) (call $const-f64))

	;; Multiple return
	;; (func $type-i32-i64 (export "type-i32-i64") (result i32 i64) (call $const-i32-i64))

	(func $type-first-i32 (export "type-first-i32") (result i32) (call $id-i32 (i32.const 32)))
	(func $type-first-i64 (export "type-first-i64") (result i64) (call $id-i64 (i64.const 64)))
	(func $type-first-f32 (export "type-first-f32") (result f32) (call $id-f32 (f32.const 1.32)))
	(func $type-first-f64 (export "type-first-f64") (result f64) (call $id-f64 (f64.const 1.64)))

	(func $type-second-i32 (export "type-second-i32") (result i32)
		(call $f32-i32 (f32.const 32.1) (i32.const 32))
	)

	(func $type-second-i64 (export "type-second-i64") (result i64)
		(call $i32-i64 (i32.const 32) (i64.const 64))
	)

	(func $type-second-f32 (export "type-second-f32") (result f32)
		(call $f64-f32 (f64.const 64) (f32.const 32))
	)

	(func $type-second-f64 (export "type-second-f64") (result f64)
		(call $i64-f64 (i64.const 64) (f64.const 64.1))
	)

	;; error: multiple result values not currently supported.
	;; (func $type-all-i32-f64 (export "type-all-i32-f64") (result i32 f64)
	;; 	(call $id-i32-f64 (i32.const 32) (f64.const 1.64))
	;; )

	;; error: multiple result values not currently supported.
	;; (func $type-all-i32-i32 (export "type-all-i32-i32") (result i32 i32)
	;; 	(call $swap-i32-i32 (i32.const 1) (i32.const 2))
	;; )

	;; error: multiple result values not currently supported.
	;; (func $type-all-f32-f64 (export "type-all-f32-f64") (result f64 f32)
	;; 	(call $swap-f32-f64 (f32.const 1) (f64.const 2))
	;; )

	;; error: multiple result values not currently supported.
	;; (func $type-all-f64-i32 (export "type-all-f64-i32") (result i32 f64)
	;; 	(call $swap-f64-i32 (f64.const 1) (i32.const 2))
	;; )

	;; Composition

	;; error: multiple result values not currently supported.
	;; (func $as-binary-all-operands (export "as-binary-all-operands") (result i32)
	;; 	(i32.add (call $swap-i32-i32 (i32.const 3) (i32.const 4)))
	;; )

	;; error: multiple result values not currently supported.
	;; (func $as-mixed-operands (export "as-mixed-operands") (result i32)
	;; 	(call $swap-i32-i32 (i32.const 3) (i32.const 4))
	;; 	(i32.const 5)
	;; 	(i32.add)
	;; 	(i32.mul)
	;; )

	;; error: multiple result values not currently supported.
	;; (func $as-call-all-operands (export "as-call-all-operands") (result i32 i32)
	;; 	(call $swap-i32-i32 (call $swap-i32-i32 (i32.const 3) (i32.const 4)))
	;; )

	;; Recursion

	(func $fac (export "fac") (param i64) (result i64)
		(if (result i64) (i64.eqz (local.get 0))
			(then (i64.const 1))
			(else
				(i64.mul
					(local.get 0)
					(call $fac (i64.sub (local.get 0) (i64.const 1)))
				)
			)
		)
	)

	(func $fac-acc (export "fac-acc") (param i64 i64) (result i64)
		(if (result i64) (i64.eqz (local.get 0))
			(then (local.get 1))
			(else
				(call $fac-acc
					(i64.sub (local.get 0) (i64.const 1))
					(i64.mul (local.get 0) (local.get 1))
				)
			)
		)
	)

	(func $fib (export "fib") (param i64) (result i64)
		(if (result i64) (i64.le_u (local.get 0) (i64.const 1))
			(then (i64.const 1))
			(else
				(i64.add
					(call $fib (i64.sub (local.get 0) (i64.const 2)))
					(call $fib (i64.sub (local.get 0) (i64.const 1)))
				)
			)
		)
	)

	(func $even (export "even") (param i64) (result i32)
		(if (result i32) (i64.eqz (local.get 0))
			(then (i32.const 44))
			(else (call $odd (i64.sub (local.get 0) (i64.const 1))))
		)
	)

	(func $odd (export "odd") (param i64) (result i32)
		(if (result i32) (i64.eqz (local.get 0))
			(then (i32.const 99))
			(else (call $even (i64.sub (local.get 0) (i64.const 1))))
		)
	)

	;; Stack exhaustion

	;; Implementations are required to have every call consume some abstract
	;; resource towards exhausting some abstract finite limit, such that
	;; infinitely recursive test cases reliably trap in finite time. This is
	;; because otherwise applications could come to depend on it on those
	;; implementations and be incompatible with implementations that don't do
	;; it (or don't do it under the same circumstances).

	(func $runaway (export "runaway") (call $runaway))

	(func $mutual-runaway1 (export "mutual-runaway") (call $mutual-runaway2))
	(func $mutual-runaway2 (call $mutual-runaway1))

	;; As parameter of control constructs and instructions

	(memory 1)

	(func $as-select-first (export "as-select-first") (result i32)
		(select (call $const-i32) (i32.const 2) (i32.const 3))
	)

	(func $as-select-mid (export "as-select-mid") (result i32)
		(select (i32.const 2) (call $const-i32) (i32.const 3))
	)

	(func $as-select-last (export "as-select-last") (result i32)
    	(select (i32.const 2) (i32.const 3) (call $const-i32))
	)

	(func $as-if-condition (export "as-if-condition") (result i32)
		(if (result i32) (call $const-i32) (then (i32.const 1)) (else (i32.const 2)))
	)

	(func $as-br_if-first (export "as-br_if-first") (result i32)
		(block (result i32) (br_if 0 (call $const-i32) (i32.const 2)))
	)

	(func $as-br_if-last (export "as-br_if-last") (result i32)
		(block (result i32) (br_if 0 (i32.const 2) (call $const-i32)))
	)

	(func $as-br_table-first (export "as-br_table-first") (result i32)
		(block (result i32) (call $const-i32) (i32.const 2) (br_table 0 0))
	)

	(func $as-br_table-last (export "as-br_table-last") (result i32)
		(block (result i32) (i32.const 2) (call $const-i32) (br_table 0 0))
	)

	(func $func (param i32 i32) (result i32) (local.get 0))

	(type $check (func (param i32 i32) (result i32)))
	
	(table funcref (elem $func))

	(func $as-call_indirect-first (export "as-call_indirect-first") (result i32)
		(block (result i32)
			(call_indirect (type $check)
				(call $const-i32) (i32.const 2) (i32.const 0)
			)
		)
	)

	(func $as-call_indirect-mid (export "as-call_indirect-mid") (result i32)
		(block (result i32)
			(call_indirect (type $check)
				(i32.const 2) (call $const-i32) (i32.const 0)
			)
		)
	)

	(func $as-call_indirect-last (export "as-call_indirect-last") (result i32)
		(block (result i32)
			(call_indirect (type $check)
				(i32.const 1) (i32.const 2) (call $const-i32)
			)
		)
	)

	(func $as-store-first (export "as-store-first")
		(call $const-i32) (i32.const 1) (i32.store)
	)

	(func $as-store-last (export "as-store-last")
		(i32.const 10) (call $const-i32) (i32.store)
	)

	(func $as-memory.grow-value (export "as-memory.grow-value") (result i32)
		(memory.grow (call $const-i32))
	)

	(func $as-return-value (export "as-return-value") (result i32)
		(call $const-i32) (return)
	)

	(func $as-drop-operand (export "as-drop-operand")
		(call $const-i32) (drop)
	)

	(func $as-br-value (export "as-br-value") (result i32)
		(block (result i32) (br 0 (call $const-i32)))
	)

	(func $as-local.set-value (export "as-local.set-value") (result i32)
		(local i32) (local.set 0 (call $const-i32)) (local.get 0)
	)

	(func $as-local.tee-value (export "as-local.tee-value") (result i32)
		(local i32) (local.tee 0 (call $const-i32))
	)

	(global $a (mut i32) (i32.const 10))
	(func $as-global.set-value (export "as-global.set-value") (result i32)
		(global.set $a (call $const-i32))
		(global.get $a)
	)

	(func $as-load-operand (export "as-load-operand") (result i32)
		(i32.load (call $const-i32))
	)

	(func $dummy (param i32) (result i32) (local.get 0))
	(func $du (param f32) (result f32) (local.get 0))
	(func $as-unary-operand (export "as-unary-operand") (result f32)
		(block (result f32) (f32.sqrt (call $du (f32.const 0x0p+0))))
	)

	(func $as-binary-left (export "as-binary-left") (result i32)
		(block (result i32) (i32.add (call $dummy (i32.const 1)) (i32.const 10)))
	)

	(func $as-binary-right (export "as-binary-right") (result i32)
		(block (result i32) (i32.sub (i32.const 10) (call $dummy (i32.const 1))))
	)

	(func $as-test-operand (export "as-test-operand") (result i32)
		(block (result i32) (i32.eqz (call $dummy (i32.const 1))))
	)

	(func $as-compare-left (export "as-compare-left") (result i32)
		(block (result i32) (i32.le_u (call $dummy (i32.const 1)) (i32.const 10)))
	)

	(func $as-compare-right (export "as-compare-right") (result i32)
		(block (result i32) (i32.ne (i32.const 10) (call $dummy (i32.const 1))))
	)

	(func $as-convert-operand (export "as-convert-operand") (result i64)
		(block (result i64) (i64.extend_i32_s (call $dummy (i32.const 1))))
	)

	;; Test correct argument passing

	(func $return-from-long-argument-list-helper (param f32 i32 i32 f64 f32 f32 f32 f64 f32 i32 i32 f32 f64 i64 i64 i32 i64 i64 f32 i64 i64 i64 i32 f32 f32 f32 f64 f32 i32 i64 f32 f64 f64 f32 i32 f32 f32 f64 i64 f64 i32 i64 f32 f64 i32 i32 i32 i64 f64 i32 i64 i64 f64 f64 f64 f64 f64 f64 i32 f32 f64 f64 i32 i64 f32 f32 f32 i32 f64 f64 f64 f64 f64 f32 i64 i64 i32 i32 i32 f32 f64 i32 i64 f32 f32 f32 i32 i32 f32 f64 i64 f32 f64 f32 f32 f32 i32 f32 i64 i32) (result i32)
		(local.get 99)
	)

	(func $return-from-long-argument-list (export "return-from-long-argument-list") (param i32) (result i32)
		(call $return-from-long-argument-list-helper (f32.const 0) (i32.const 0) (i32.const 0) (f64.const 0) (f32.const 0) (f32.const 0) (f32.const 0) (f64.const 0) (f32.const 0) (i32.const 0) (i32.const 0) (f32.const 0) (f64.const 0) (i64.const 0) (i64.const 0) (i32.const 0) (i64.const 0) (i64.const 0) (f32.const 0) (i64.const 0) (i64.const 0) (i64.const 0) (i32.const 0) (f32.const 0) (f32.const 0) (f32.const 0) (f64.const 0) (f32.const 0) (i32.const 0) (i64.const 0) (f32.const 0) (f64.const 0) (f64.const 0) (f32.const 0) (i32.const 0) (f32.const 0) (f32.const 0) (f64.const 0) (i64.const 0) (f64.const 0) (i32.const 0) (i64.const 0) (f32.const 0) (f64.const 0) (i32.const 0) (i32.const 0) (i32.const 0) (i64.const 0) (f64.const 0) (i32.const 0) (i64.const 0) (i64.const 0) (f64.const 0) (f64.const 0) (f64.const 0) (f64.const 0) (f64.const 0) (f64.const 0) (i32.const 0) (f32.const 0) (f64.const 0) (f64.const 0) (i32.const 0) (i64.const 0) (f32.const 0) (f32.const 0) (f32.const 0) (i32.const 0) (f64.const 0) (f64.const 0) (f64.const 0) (f64.const 0) (f64.const 0) (f32.const 0) (i64.const 0) (i64.const 0) (i32.const 0) (i32.const 0) (i32.const 0) (f32.const 0) (f64.const 0) (i32.const 0) (i64.const 0) (f32.const 0) (f32.const 0) (f32.const 0) (i32.const 0) (i32.const 0) (f32.const 0) (f64.const 0) (i64.const 0) (f32.const 0) (f64.const 0) (f32.const 0) (f32.const 0) (f32.const 0) (i32.const 0) (f32.const 0) (i64.const 0) (local.get 0))
	)

	(func (export "_start") (param i32)

		(if (i32.ne (call $type-i32) (i32.const 0x132)) (then
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $type-i64) (i64.const 0x164)) (then
			(call $proc_exit (i32.const 2))
		))

		(if (f32.ne (call $type-f32) (f32.const 0xf32)) (then
			(call $proc_exit (i32.const 3))
		))

		(if (f64.ne (call $type-f64) (f64.const 0xf64)) (then
			(call $proc_exit (i32.const 4))
		))

		;; (call $type-i32-i64) (i32.const 0x132) (i64.const 0x164))

		(if (i32.ne (call $type-first-i32) (i32.const 32)) (then
			(call $proc_exit (i32.const 6))
		))

		(if (i64.ne (call $type-first-i64) (i64.const 64)) (then
			(call $proc_exit (i32.const 7))
		))

		(if (f32.ne (call $type-first-f32) (f32.const 1.32)) (then
			(call $proc_exit (i32.const 8))
		))

		(if (f64.ne (call $type-first-f64) (f64.const 1.64)) (then
			(call $proc_exit (i32.const 9))
		))

		(if (i32.ne (call $type-second-i32) (i32.const 32)) (then
			(call $proc_exit (i32.const 10))
		))

		(if (i64.ne (call $type-second-i64) (i64.const 64)) (then
			(call $proc_exit (i32.const 11))
		))

		(if (f32.ne (call $type-second-f32) (f32.const 32)) (then
			(call $proc_exit (i32.const 12))
		))

		(if (f64.ne (call $type-second-f64) (f64.const 64.1)) (then
			(call $proc_exit (i32.const 13))
		))

		;; (assert_return (invoke "type-all-i32-f64") (i32.const 32) (f64.const 1.64))
		;; (assert_return (invoke "type-all-i32-i32") (i32.const 2) (i32.const 1))
		;; (assert_return (invoke "type-all-f32-f64") (f64.const 2) (f32.const 1))
		;; (assert_return (invoke "type-all-f64-i32") (i32.const 2) (f64.const 1))

		;; (if (i32.ne (call $as-binary-all-operands) (i32.const 7)) (then
		;; 	(call $proc_exit (i32.const 18))
		;; ))

		;; (if (i32.ne (call $as-mixed-operands) (i32.const 32)) (then
		;; 	(call $proc_exit (i32.const 19))
		;; ))
		
		;; (assert_return (invoke "as-call-all-operands") (i32.const 3) (i32.const 4))

		(if (i64.ne (call $fac (i64.const 0)) (i64.const 1)) (then
			(call $proc_exit (i32.const 21))
		))

		(if (i64.ne (call $fac (i64.const 1)) (i64.const 1)) (then
			(call $proc_exit (i32.const 22))
		))

		(if (i64.ne (call $fac (i64.const 5)) (i64.const 120)) (then
			(call $proc_exit (i32.const 23))
		))

		(if (i64.ne (call $fac (i64.const 25)) (i64.const 7034535277573963776)) (then
			(call $proc_exit (i32.const 24))
		))
	
		(if (i64.ne (call $fac-acc (i64.const 0) (i64.const 1)) (i64.const 1)) (then
			(call $proc_exit (i32.const 25))
		))

		(if (i64.ne (call $fac-acc (i64.const 1) (i64.const 1)) (i64.const 1)) (then
			(call $proc_exit (i32.const 26))
		))

		(if (i64.ne (call $fac-acc (i64.const 5) (i64.const 1)) (i64.const 120)) (then
			(call $proc_exit (i32.const 27))
		))

		(if (i64.ne (call $fac-acc (i64.const 25) (i64.const 1)) (i64.const 7034535277573963776)) (then
			(call $proc_exit (i32.const 28))
		))

		(if (i64.ne (call $fib (i64.const 0)) (i64.const 1)) (then
			(call $proc_exit (i32.const 29))
		))

		(if (i64.ne (call $fib (i64.const 1)) (i64.const 1)) (then
			(call $proc_exit (i32.const 30))
		))

		(if (i64.ne (call $fib (i64.const 2)) (i64.const 2)) (then
			(call $proc_exit (i32.const 31))
		))

		(if (i64.ne (call $fib (i64.const 5)) (i64.const 8)) (then
			(call $proc_exit (i32.const 32))
		))

		(if (i64.ne (call $fib (i64.const 20)) (i64.const 10946)) (then
			(call $proc_exit (i32.const 33))
		))

		(if (i32.ne (call $even (i64.const 0)) (i32.const 44)) (then
			(call $proc_exit (i32.const 34))
		))

		(if (i32.ne (call $even (i64.const 1)) (i32.const 99)) (then
			(call $proc_exit (i32.const 35))
		))

		(if (i32.ne (call $even (i64.const 100)) (i32.const 44)) (then
			(call $proc_exit (i32.const 36))
		))

		(if (i32.ne (call $even (i64.const 77)) (i32.const 99)) (then
			(call $proc_exit (i32.const 37))
		))

		(if (i32.ne (call $odd (i64.const 0)) (i32.const 99)) (then
			(call $proc_exit (i32.const 38))
		))

		(if (i32.ne (call $odd (i64.const 1)) (i32.const 44)) (then
			(call $proc_exit (i32.const 39))
		))

		(if (i32.ne (call $odd (i64.const 200)) (i32.const 99)) (then
			(call $proc_exit (i32.const 40))
		))

		(if (i32.ne (call $odd (i64.const 77)) (i32.const 44)) (then
			(call $proc_exit (i32.const 41))
		))


		;; (assert_exhaustion (invoke "runaway") "call stack exhausted")
		;; (assert_exhaustion (invoke "mutual-runaway") "call stack exhausted")

		(if (i32.ne (call $as-select-first) (i32.const 0x132)) (then
			(call $proc_exit (i32.const 44))
		))

		(if (i32.ne (call $as-select-mid) (i32.const 2)) (then
			(call $proc_exit (i32.const 45))
		))
		
		(if (i32.ne (call $as-select-last) (i32.const 2)) (then
			(call $proc_exit (i32.const 46))
		))

		(if (i32.ne (call $as-if-condition) (i32.const 1)) (then
			(call $proc_exit (i32.const 47))
		))

		(if (i32.ne (call $as-br_if-first) (i32.const 0x132)) (then
			(call $proc_exit (i32.const 48))
		))

		(if (i32.ne (call $as-br_if-first) (i32.const 2)) (then
			(call $proc_exit (i32.const 49))
		))

		(if (i32.ne (call $as-br_if-last) (i32.const 2)) (then
			(call $proc_exit (i32.const 50))
		))

		(if (i32.ne (call $as-br_table-first) (i32.const 0x132)) (then
			(call $proc_exit (i32.const 51))
		))

		(if (i32.ne (call $as-br_table-last) (i32.const 2)) (then
			(call $proc_exit (i32.const 52))
		))

		(if (i32.ne (call $as-call_indirect-first) (i32.const 0x132)) (then
			(call $proc_exit (i32.const 53))
		))

		(if (i32.ne (call $as-call_indirect-mid) (i32.const 2)) (then
			(call $proc_exit (i32.const 54))
		))

		;; (assert_trap (invoke "as-call_indirect-last") "undefined element")

		(call $as-store-first)
		(call $as-store-last)

		(if (i32.ne (call $as-memory.grow-value) (i32.const 1)) (then
			(call $proc_exit (i32.const 56))
		))

		(if (i32.ne (call $as-return-value) (i32.const 0x132)) (then
			(call $proc_exit (i32.const 57))
		))

		(if (i32.ne (call $as-br-value) (i32.const 0x132)) (then
			(call $proc_exit (i32.const 58))
		))

		(call $as-drop-operand)

		(if (i32.ne (call $as-local.set-value) (i32.const 0x132)) (then
			(call $proc_exit (i32.const 59))
		))

		(if (i32.ne (call $as-local.tee-value) (i32.const 0x132)) (then
			(call $proc_exit (i32.const 60))
		))

		(if (i32.ne (call $as-global.set-value) (i32.const 0x132)) (then
			(call $proc_exit (i32.const 61))
		))

		(if (i32.ne (call $as-load-operand) (i32.const 0x132)) (then
			(call $proc_exit (i32.const 62))
		))

		(if (f32.ne (call $as-unary-operand) (f32.const 0x0p+0)) (then
			(call $proc_exit (i32.const 63))
		))

		(if (i32.ne (call $as-binary-left) (i32.const 11)) (then
			(call $proc_exit (i32.const 64))
		))

		(if (i32.ne (call $as-binary-right) (i32.const 9)) (then
			(call $proc_exit (i32.const 65))
		))

		(if (i32.ne (call $as-test-operand) (i32.const 0)) (then
			(call $proc_exit (i32.const 66))
		))

		(if (i32.ne (call $as-compare-left) (i32.const 1)) (then
			(call $proc_exit (i32.const 67))
		))

		(if (i32.ne (call $as-compare-right) (i32.const 1)) (then
			(call $proc_exit (i32.const 68))
		))

		(if (i64.ne (call $as-convert-operand) (i64.const 1)) (then
			(call $proc_exit (i32.const 69))
		))

		(if (i32.ne (call $return-from-long-argument-list (i32.const 42)) (i32.const 42)) (then
			(call $proc_exit (i32.const 70))
		))

		(call $proc_exit (i32.const 0))
		
	)
)

;; Invalid typing

;; (assert_invalid
;;   (module
;;     (func $type-void-vs-num (i32.eqz (call 1)))
;;     (func)
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-num-vs-num (i32.eqz (call 1)))
;;     (func (result i64) (i64.const 1))
;;   )
;;   "type mismatch"
;; )

;; (assert_invalid
;;   (module
;;     (func $arity-0-vs-1 (call 1))
;;     (func (param i32))
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $arity-0-vs-2 (call 1))
;;     (func (param f64 i32))
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $arity-1-vs-0 (call 1 (i32.const 1)))
;;     (func)
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $arity-2-vs-0 (call 1 (f64.const 2) (i32.const 1)))
;;     (func)
;;   )
;;   "type mismatch"
;; )

;; (assert_invalid
;;   (module
;;     (func $type-first-void-vs-num (call 1 (nop) (i32.const 1)))
;;     (func (param i32 i32))
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-second-void-vs-num (call 1 (i32.const 1) (nop)))
;;     (func (param i32 i32))
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-first-num-vs-num (call 1 (f64.const 1) (i32.const 1)))
;;     (func (param i32 f64))
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-second-num-vs-num (call 1 (i32.const 1) (f64.const 1)))
;;     (func (param f64 i32))
;;   )
;;   "type mismatch"
;; )

;; (assert_invalid
;;   (module
;;     (func $type-first-empty-in-block
;;       (block (call 1))
;;     )
;;     (func (param i32))
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-second-empty-in-block
;;       (block (call 1 (i32.const 0)))
;;     )
;;     (func (param i32 i32))
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-first-empty-in-loop
;;       (loop (call 1))
;;     )
;;     (func (param i32))
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-second-empty-in-loop
;;       (loop (call 1 (i32.const 0)))
;;     )
;;     (func (param i32 i32))
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-first-empty-in-then
;;       (if (i32.const 0) (then (call 1)))
;;     )
;;     (func (param i32))
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-second-empty-in-then
;;       (if (i32.const 0) (then (call 1 (i32.const 0))))
;;     )
;;     (func (param i32 i32))
;;   )
;;   "type mismatch"
;; )


;; Unbound function

;; (assert_invalid
;;   (module (func $unbound-func (call 1)))
;;   "unknown function"
;; )
;; (assert_invalid
;;   (module (func $large-func (call 1012321300)))
;;   "unknown function"
;; )
