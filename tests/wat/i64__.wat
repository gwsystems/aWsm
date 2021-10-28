;; i64 operations

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
    (export "memory" (memory 0))

	(func $add (export "add") (param $x i64) (param $y i64) (result i64) (i64.add (local.get $x) (local.get $y)))
	(func $sub (export "sub") (param $x i64) (param $y i64) (result i64) (i64.sub (local.get $x) (local.get $y)))
	(func $mul (export "mul") (param $x i64) (param $y i64) (result i64) (i64.mul (local.get $x) (local.get $y)))
	(func $div_s (export "div_s") (param $x i64) (param $y i64) (result i64) (i64.div_s (local.get $x) (local.get $y)))
	(func $div_u (export "div_u") (param $x i64) (param $y i64) (result i64) (i64.div_u (local.get $x) (local.get $y)))
	(func $rem_u (export "rem_u") (param $x i64) (param $y i64) (result i64) (i64.rem_u (local.get $x) (local.get $y)))
	(func $and (export "and") (param $x i64) (param $y i64) (result i64) (i64.and (local.get $x) (local.get $y)))
	(func $or (export "or") (param $x i64) (param $y i64) (result i64) (i64.or (local.get $x) (local.get $y)))
	(func $xor (export "xor") (param $x i64) (param $y i64) (result i64) (i64.xor (local.get $x) (local.get $y)))
	(func $shl (export "shl") (param $x i64) (param $y i64) (result i64) (i64.shl (local.get $x) (local.get $y)))
	(func $shr_s (export "shr_s") (param $x i64) (param $y i64) (result i64) (i64.shr_s (local.get $x) (local.get $y)))
	(func $shr_u(export "shr_u") (param $x i64) (param $y i64) (result i64) (i64.shr_u (local.get $x) (local.get $y)))
	(func $rotl (export "rotl") (param $x i64) (param $y i64) (result i64) (i64.rotl (local.get $x) (local.get $y)))
	(func $rotr (export "rotr") (param $x i64) (param $y i64) (result i64) (i64.rotr (local.get $x) (local.get $y)))
	(func $clz (export "clz") (param $x i64) (result i64) (i64.clz (local.get $x)))
	(func $popcnt (export "popcnt") (param $x i64) (result i64) (i64.popcnt (local.get $x)))
	;;  error: opcode not allowed: i64.extend8_s
	;; (func $extend8_s (export "extend8_s") (param $x i64) (result i64) (i64.extend8_s (local.get $x)))
	;; error: opcode not allowed: i64.extend16_s
	;; (func $extend16_s (export "extend16_s") (param $x i64) (result i64) (i64.extend16_s (local.get $x)))
	;; opcode not allowed: i64.extend32_s
	;; (func $extend32_s (export "extend32_s") (param $x i64) (result i64) (i64.extend32_s (local.get $x)))
	(func $eqz (export "eqz") (param $x i64) (result i32) (i64.eqz (local.get $x)))
	(func $eq (export "eq") (param $x i64) (param $y i64) (result i32) (i64.eq (local.get $x) (local.get $y)))
	(func $ne (export "ne") (param $x i64) (param $y i64) (result i32) (i64.ne (local.get $x) (local.get $y)))
	(func $lt_s (export "lt_s") (param $x i64) (param $y i64) (result i32) (i64.lt_s (local.get $x) (local.get $y)))
	(func $lt_u (export "lt_u") (param $x i64) (param $y i64) (result i32) (i64.lt_u (local.get $x) (local.get $y)))
	(func $le_s (export "le_s") (param $x i64) (param $y i64) (result i32) (i64.le_s (local.get $x) (local.get $y)))
	(func $le_u (export "le_u") (param $x i64) (param $y i64) (result i32) (i64.le_u (local.get $x) (local.get $y)))
	(func $gt_s (export "gt_s") (param $x i64) (param $y i64) (result i32) (i64.gt_s (local.get $x) (local.get $y)))
	(func $gt_u (export "gt_u") (param $x i64) (param $y i64) (result i32) (i64.gt_u (local.get $x) (local.get $y)))
	(func $ge_s (export "ge_s") (param $x i64) (param $y i64) (result i32) (i64.ge_s (local.get $x) (local.get $y)))
	(func $ge_u (export "ge_u") (param $x i64) (param $y i64) (result i32) (i64.ge_u (local.get $x) (local.get $y)))

	(func $_start (export "_start")
		(if (i64.ne (call $add (i64.const 1) (i64.const 1)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $add (i64.const 1) (i64.const 0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $add (i64.const -1) (i64.const -1)) (i64.const -2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $add (i64.const -1) (i64.const 1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $add (i64.const 0x7fffffffffffffff) (i64.const 1)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $add (i64.const 0x8000000000000000) (i64.const -1)) (i64.const 0x7fffffffffffffff)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $add (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $add (i64.const 0x3fffffff) (i64.const 1)) (i64.const 0x40000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $sub (i64.const 1) (i64.const 1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $sub (i64.const 1) (i64.const 0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $sub (i64.const -1) (i64.const -1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $sub (i64.const 0x7fffffffffffffff) (i64.const -1)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $sub (i64.const 0x8000000000000000) (i64.const 1)) (i64.const 0x7fffffffffffffff)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $sub (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $sub (i64.const 0x3fffffff) (i64.const -1)) (i64.const 0x40000000)) (then     
			(call $proc_exit (i32.const 1))
		))
	
		(if (i64.ne (call $mul (i64.const 1) (i64.const 1)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $mul (i64.const 1) (i64.const 0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $mul (i64.const -1) (i64.const -1)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $mul (i64.const 0x1000000000000000) (i64.const 4096)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $mul (i64.const 0x8000000000000000) (i64.const 0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $mul (i64.const 0x8000000000000000) (i64.const -1)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $mul (i64.const 0x7fffffffffffffff) (i64.const -1)) (i64.const 0x8000000000000001)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $mul (i64.const 0x0123456789abcdef) (i64.const 0xfedcba9876543210)) (i64.const 0x2236d88fe5618cf0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $mul (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		;; (assert_trap (invoke "div_s" (i64.const 1) (i64.const 0)) "integer divide by zero")
		;; (assert_trap (invoke "div_s" (i64.const 0) (i64.const 0)) "integer divide by zero")
		;; (assert_trap (invoke "div_s" (i64.const 0x8000000000000000) (i64.const -1)) "integer overflow")
		;; (assert_trap (invoke "div_s" (i64.const 0x8000000000000000) (i64.const 0)) "integer divide by zero")

		(if (i64.ne (call $div_s (i64.const 1) (i64.const 1)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const 0) (i64.const 1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const 0) (i64.const -1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const -1) (i64.const -1)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const 0x8000000000000000) (i64.const 2)) (i64.const 0xc000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const 0x8000000000000001) (i64.const 1000)) (i64.const 0xffdf3b645a1cac09)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const 5) (i64.const 2)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const -5) (i64.const 2)) (i64.const -2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const 5) (i64.const -2)) (i64.const -2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const -5) (i64.const -2)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const 7) (i64.const 3)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const -7) (i64.const 3)) (i64.const -2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const 7) (i64.const -3)) (i64.const -2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const -7) (i64.const -3)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const 11) (i64.const 5)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_s (i64.const 17) (i64.const 7)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		;; (assert_trap (invoke "div_u" (i64.const 1) (i64.const 0)) "integer divide by zero")
		;; (assert_trap (invoke "div_u" (i64.const 0) (i64.const 0)) "integer divide by zero")

		(if (i64.ne (call $div_u (i64.const 1) (i64.const 1)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_u (i64.const 0) (i64.const 1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_u (i64.const -1) (i64.const -1)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_u (i64.const 0x8000000000000000) (i64.const -1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_u (i64.const 0x8000000000000000) (i64.const 2)) (i64.const 0x4000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_u (i64.const 0x8ff00ff00ff00ff0) (i64.const 0x100000001)) (i64.const 0x8ff00fef)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_u (i64.const 0x8000000000000001) (i64.const 1000)) (i64.const 0x20c49ba5e353f7)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_u (i64.const 5) (i64.const 2)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_u (i64.const -5) (i64.const 2)) (i64.const 0x7ffffffffffffffd)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_u (i64.const 5) (i64.const -2)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_u (i64.const -5) (i64.const -2)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_u (i64.const 7) (i64.const 3)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_u (i64.const 11) (i64.const 5)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $div_u (i64.const 17) (i64.const 7)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		;; (assert_trap (invoke "rem_u" (i64.const 1) (i64.const 0)) "integer divide by zero")
		;; (assert_trap (invoke "rem_u" (i64.const 0) (i64.const 0)) "integer divide by zero")

		(if (i64.ne (call $rem_u (i64.const 1) (i64.const 1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_u (i64.const 0) (i64.const 1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_u (i64.const -1) (i64.const -1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_u (i64.const 0x8000000000000000) (i64.const -1)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_u (i64.const 0x8000000000000000) (i64.const 2)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_u (i64.const 0x8ff00ff00ff00ff0) (i64.const 0x100000001)) (i64.const 0x80000001)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_u (i64.const 0x8000000000000001) (i64.const 1000)) (i64.const 809)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_u (i64.const 5) (i64.const 2)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_u (i64.const -5) (i64.const 2)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_u (i64.const 5) (i64.const -2)) (i64.const 5)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_u (i64.const -5) (i64.const -2)) (i64.const -5)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_u (i64.const 7) (i64.const 3)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_u (i64.const 11) (i64.const 5)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rem_u (i64.const 17) (i64.const 7)) (i64.const 3)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $and (i64.const 1) (i64.const 0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $and (i64.const 0) (i64.const 1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $and (i64.const 1) (i64.const 1)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $and (i64.const 0) (i64.const 0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $and (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $and (i64.const 0x7fffffffffffffff) (i64.const -1)) (i64.const 0x7fffffffffffffff)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $and (i64.const 0xf0f0ffff) (i64.const 0xfffff0f0)) (i64.const 0xf0f0f0f0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $and (i64.const 0xffffffffffffffff) (i64.const 0xffffffffffffffff)) (i64.const 0xffffffffffffffff)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $or (i64.const 1) (i64.const 0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $or (i64.const 0) (i64.const 1)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $or (i64.const 1) (i64.const 1)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $or (i64.const 0) (i64.const 0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $or (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $or (i64.const 0x8000000000000000) (i64.const 0)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $or (i64.const 0xf0f0ffff) (i64.const 0xfffff0f0)) (i64.const 0xffffffff)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $or (i64.const 0xffffffffffffffff) (i64.const 0xffffffffffffffff)) (i64.const 0xffffffffffffffff)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $xor (i64.const 1) (i64.const 0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $xor (i64.const 0) (i64.const 1)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $xor (i64.const 1) (i64.const 1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $xor (i64.const 0) (i64.const 0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $xor (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $xor (i64.const 0x8000000000000000) (i64.const 0)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $xor (i64.const -1) (i64.const 0x8000000000000000)) (i64.const 0x7fffffffffffffff)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $xor (i64.const -1) (i64.const 0x7fffffffffffffff)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $xor (i64.const 0xf0f0ffff) (i64.const 0xfffff0f0)) (i64.const 0x0f0f0f0f)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $xor (i64.const 0xffffffffffffffff) (i64.const 0xffffffffffffffff)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shl (i64.const 1) (i64.const 1)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shl (i64.const 1) (i64.const 0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shl (i64.const 0x7fffffffffffffff) (i64.const 1)) (i64.const 0xfffffffffffffffe)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shl (i64.const 0xffffffffffffffff) (i64.const 1)) (i64.const 0xfffffffffffffffe)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shl (i64.const 0x8000000000000000) (i64.const 1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shl (i64.const 0x4000000000000000) (i64.const 1)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shl (i64.const 1) (i64.const 63)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shl (i64.const 1) (i64.const 64)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shl (i64.const 1) (i64.const 65)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shl (i64.const 1) (i64.const -1)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shl (i64.const 1) (i64.const 0x7fffffffffffffff)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const 1) (i64.const 1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const 1) (i64.const 0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const -1) (i64.const 1)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const 0x7fffffffffffffff) (i64.const 1)) (i64.const 0x3fffffffffffffff)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const 0x8000000000000000) (i64.const 1)) (i64.const 0xc000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const 0x4000000000000000) (i64.const 1)) (i64.const 0x2000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const 1) (i64.const 64)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const 1) (i64.const 65)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const 1) (i64.const -1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const 1) (i64.const 0x7fffffffffffffff)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const 1) (i64.const 0x8000000000000000)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const 0x8000000000000000) (i64.const 63)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const -1) (i64.const 64)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const -1) (i64.const 65)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const -1) (i64.const -1)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const -1) (i64.const 0x7fffffffffffffff)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_s (i64.const -1) (i64.const 0x8000000000000000)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const 1) (i64.const 1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const 1) (i64.const 0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const -1) (i64.const 1)) (i64.const 0x7fffffffffffffff)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const 0x7fffffffffffffff) (i64.const 1)) (i64.const 0x3fffffffffffffff)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const 0x8000000000000000) (i64.const 1)) (i64.const 0x4000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const 0x4000000000000000) (i64.const 1)) (i64.const 0x2000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const 1) (i64.const 64)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const 1) (i64.const 65)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const 1) (i64.const -1)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const 1) (i64.const 0x7fffffffffffffff)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const 1) (i64.const 0x8000000000000000)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const 0x8000000000000000) (i64.const 63)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const -1) (i64.const 64)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const -1) (i64.const 65)) (i64.const 0x7fffffffffffffff)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const -1) (i64.const -1)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const -1) (i64.const 0x7fffffffffffffff)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $shr_u (i64.const -1) (i64.const 0x8000000000000000)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotl (i64.const 1) (i64.const 1)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotl (i64.const 1) (i64.const 0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotl (i64.const -1) (i64.const 1)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotl (i64.const 1) (i64.const 64)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotl (i64.const 0xabcd987602468ace) (i64.const 1)) (i64.const 0x579b30ec048d159d)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotl (i64.const 0xfe000000dc000000) (i64.const 4)) (i64.const 0xe000000dc000000f)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotl (i64.const 0xabcd1234ef567809) (i64.const 53)) (i64.const 0x013579a2469deacf)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotl (i64.const 0xabd1234ef567809c) (i64.const 63)) (i64.const 0x55e891a77ab3c04e)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotl (i64.const 0xabcd1234ef567809) (i64.const 0xf5)) (i64.const 0x013579a2469deacf)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotl (i64.const 0xabcd7294ef567809) (i64.const 0xffffffffffffffed)) (i64.const 0xcf013579ae529dea)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotl (i64.const 0xabd1234ef567809c) (i64.const 0x800000000000003f)) (i64.const 0x55e891a77ab3c04e)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotl (i64.const 1) (i64.const 63)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotl (i64.const 0x8000000000000000) (i64.const 1)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotr (i64.const 1) (i64.const 1)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotr (i64.const 1) (i64.const 0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotr (i64.const -1) (i64.const 1)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotr (i64.const 1) (i64.const 64)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotr (i64.const 0xabcd987602468ace) (i64.const 1)) (i64.const 0x55e6cc3b01234567)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotr (i64.const 0xfe000000dc000000) (i64.const 4)) (i64.const 0x0fe000000dc00000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotr (i64.const 0xabcd1234ef567809) (i64.const 53)) (i64.const 0x6891a77ab3c04d5e)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotr (i64.const 0xabd1234ef567809c) (i64.const 63)) (i64.const 0x57a2469deacf0139)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotr (i64.const 0xabcd1234ef567809) (i64.const 0xf5)) (i64.const 0x6891a77ab3c04d5e)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotr (i64.const 0xabcd7294ef567809) (i64.const 0xffffffffffffffed)) (i64.const 0x94a77ab3c04d5e6b)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotr (i64.const 0xabd1234ef567809c) (i64.const 0x800000000000003f)) (i64.const 0x57a2469deacf0139)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotr (i64.const 1) (i64.const 63)) (i64.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $rotr (i64.const 0x8000000000000000) (i64.const 63)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $clz (i64.const 0xffffffffffffffff)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $clz (i64.const 0)) (i64.const 64)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $clz (i64.const 0x00008000)) (i64.const 48)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $clz (i64.const 0xff)) (i64.const 56)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $clz (i64.const 0x8000000000000000)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $clz (i64.const 1)) (i64.const 63)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $clz (i64.const 2)) (i64.const 62)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $clz (i64.const 0x7fffffffffffffff)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $popcnt (i64.const -1)) (i64.const 64)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $popcnt (i64.const 0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $popcnt (i64.const 0x00008000)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $popcnt (i64.const 0x8000800080008000)) (i64.const 4)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $popcnt (i64.const 0x7fffffffffffffff)) (i64.const 63)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $popcnt (i64.const 0xAAAAAAAA55555555)) (i64.const 32)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $popcnt (i64.const 0x99999999AAAAAAAA)) (i64.const 32)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $popcnt (i64.const 0xDEADBEEFDEADBEEF)) (i64.const 48)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eqz (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eqz (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eqz (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eqz (i64.const 0x7fffffffffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eqz (i64.const 0xffffffffffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const 0) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const 1) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const -1) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const -1) (i64.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const 1) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const 0) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $eq (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const 0) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const 1) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const -1) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const -1) (i64.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const 1) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const 0) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const 0) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const 1) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const -1) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const -1) (i64.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const 1) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const 0) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const 0) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const 1) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const -1) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const -1) (i64.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const 1) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const 0) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const 0) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const 1) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const -1) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const -1) (i64.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const 1) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const 0) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const 0) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const 1) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const -1) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const -1) (i64.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const 1) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const 0) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const 0) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const 1) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const -1) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const -1) (i64.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const 1) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const 0) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const 0) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const 1) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const -1) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const -1) (i64.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const 1) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const 0) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const 0) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const 1) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const -1) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const -1) (i64.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const 1) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const 0) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const 0) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const 1) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const -1) (i64.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const -1) (i64.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const 1) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const 0) (i64.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(call $proc_exit (i32.const 0))

	)
)

;; (assert_return (invoke "extend8_s" (i64.const 0)) (i64.const 0))
;; (assert_return (invoke "extend8_s" (i64.const 0x7f)) (i64.const 127))
;; (assert_return (invoke "extend8_s" (i64.const 0x80)) (i64.const -128))
;; (assert_return (invoke "extend8_s" (i64.const 0xff)) (i64.const -1))
;; (assert_return (invoke "extend8_s" (i64.const 0x01234567_89abcd_00)) (i64.const 0))
;; (assert_return (invoke "extend8_s" (i64.const 0xfedcba98_765432_80)) (i64.const -0x80))
;; (assert_return (invoke "extend8_s" (i64.const -1)) (i64.const -1))

;; (assert_return (invoke "extend16_s" (i64.const 0)) (i64.const 0))
;; (assert_return (invoke "extend16_s" (i64.const 0x7fff)) (i64.const 32767))
;; (assert_return (invoke "extend16_s" (i64.const 0x8000)) (i64.const -32768))
;; (assert_return (invoke "extend16_s" (i64.const 0xffff)) (i64.const -1))
;; (assert_return (invoke "extend16_s" (i64.const 0x12345678_9abc_0000)) (i64.const 0))
;; (assert_return (invoke "extend16_s" (i64.const 0xfedcba98_7654_8000)) (i64.const -0x8000))
;; (assert_return (invoke "extend16_s" (i64.const -1)) (i64.const -1))

;; (assert_return (invoke "extend32_s" (i64.const 0)) (i64.const 0))
;; (assert_return (invoke "extend32_s" (i64.const 0x7fff)) (i64.const 32767))
;; (assert_return (invoke "extend32_s" (i64.const 0x8000)) (i64.const 32768))
;; (assert_return (invoke "extend32_s" (i64.const 0xffff)) (i64.const 65535))
;; (assert_return (invoke "extend32_s" (i64.const 0x7fffffff)) (i64.const 0x7fffffff))
;; (assert_return (invoke "extend32_s" (i64.const 0x80000000)) (i64.const -0x80000000))
;; (assert_return (invoke "extend32_s" (i64.const 0xffffffff)) (i64.const -1))
;; (assert_return (invoke "extend32_s" (i64.const 0x01234567_00000000)) (i64.const 0))
;; (assert_return (invoke "extend32_s" (i64.const 0xfedcba98_80000000)) (i64.const -0x80000000))
;; (assert_return (invoke "extend32_s" (i64.const -1)) (i64.const -1))

;; ;; Type check

;; (assert_invalid (module (func (result i64) (i64.add (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.and (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.div_s (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.div_u (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.mul (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.or (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.rem_s (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.rem_u (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.rotl (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.rotr (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.shl (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.shr_s (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.shr_u (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.sub (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.xor (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.eqz (i32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.clz (i32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.popcnt (i32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.eq (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.ge_s (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.ge_u (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.gt_s (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.gt_u (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.le_s (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.le_u (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.lt_s (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.lt_u (i32.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i64) (i64.ne (i32.const 0) (f32.const 0)))) "type mismatch")
