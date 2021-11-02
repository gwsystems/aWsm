;; i32 operations

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
    (export "memory" (memory 0))

	(func $add (export "add") (param $x i32) (param $y i32) (result i32) (i32.add (local.get $x) (local.get $y)))
	(func $sub (export "sub") (param $x i32) (param $y i32) (result i32) (i32.sub (local.get $x) (local.get $y)))
	(func $mul (export "mul") (param $x i32) (param $y i32) (result i32) (i32.mul (local.get $x) (local.get $y)))
	(func $div_s (export "div_s") (param $x i32) (param $y i32) (result i32) (i32.div_s (local.get $x) (local.get $y)))
	(func $div_u (export "div_u") (param $x i32) (param $y i32) (result i32) (i32.div_u (local.get $x) (local.get $y)))
	(func $rem_u (export "rem_u") (param $x i32) (param $y i32) (result i32) (i32.rem_u (local.get $x) (local.get $y)))
	(func $and (export "and") (param $x i32) (param $y i32) (result i32) (i32.and (local.get $x) (local.get $y)))
	(func $or (export "or") (param $x i32) (param $y i32) (result i32) (i32.or (local.get $x) (local.get $y)))
	(func $xor (export "xor") (param $x i32) (param $y i32) (result i32) (i32.xor (local.get $x) (local.get $y)))
	(func $shl (export "shl") (param $x i32) (param $y i32) (result i32) (i32.shl (local.get $x) (local.get $y)))
	(func $shr_s (export "shr_s") (param $x i32) (param $y i32) (result i32) (i32.shr_s (local.get $x) (local.get $y)))
	(func $shr_u (export "shr_u") (param $x i32) (param $y i32) (result i32) (i32.shr_u (local.get $x) (local.get $y)))
	(func $rotl (export "rotl") (param $x i32) (param $y i32) (result i32) (i32.rotl (local.get $x) (local.get $y)))
	(func $rotr (export "rotr") (param $x i32) (param $y i32) (result i32) (i32.rotr (local.get $x) (local.get $y)))
	(func $popcnt (export "popcnt") (param $x i32) (result i32) (i32.popcnt (local.get $x)))
	;; (func $extend8_s (export "extend8_s") (param $x i32) (result i32) (i32.extend8_s (local.get $x)))
	;; (func $extend16_s (export "extend16_s") (param $x i32) (result i32) (i32.extend16_s (local.get $x)))
	(func $eqz (export "eqz") (param $x i32) (result i32) (i32.eqz (local.get $x)))
	(func $eq (export "eq") (param $x i32) (param $y i32) (result i32) (i32.eq (local.get $x) (local.get $y)))
	(func $ne (export "ne") (param $x i32) (param $y i32) (result i32) (i32.ne (local.get $x) (local.get $y)))
	(func $lt_s (export "lt_s") (param $x i32) (param $y i32) (result i32) (i32.lt_s (local.get $x) (local.get $y)))
	(func $lt_u (export "lt_u") (param $x i32) (param $y i32) (result i32) (i32.lt_u (local.get $x) (local.get $y)))
	(func $le_s (export "le_s") (param $x i32) (param $y i32) (result i32) (i32.le_s (local.get $x) (local.get $y)))
	(func $le_u (export "le_u") (param $x i32) (param $y i32) (result i32) (i32.le_u (local.get $x) (local.get $y)))
	(func $gt_s (export "gt_s") (param $x i32) (param $y i32) (result i32) (i32.gt_s (local.get $x) (local.get $y)))
	(func $gt_u (export "gt_u") (param $x i32) (param $y i32) (result i32) (i32.gt_u (local.get $x) (local.get $y)))
	(func $ge_s (export "ge_s") (param $x i32) (param $y i32) (result i32) (i32.ge_s (local.get $x) (local.get $y)))
	(func $ge_u (export "ge_u") (param $x i32) (param $y i32) (result i32) (i32.ge_u (local.get $x) (local.get $y)))

	(func $_start (export "_start")
		(if (i32.ne (call $add (i32.const 1) (i32.const 1)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $add (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 2))
		))

		(if (i32.ne (call $add (i32.const -1) (i32.const -1)) (i32.const -2)) (then     
			(call $proc_exit (i32.const 3))
		))

		(if (i32.ne (call $add (i32.const -1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 4))
		))

		(if (i32.ne (call $add (i32.const 0x7fffffff) (i32.const 1)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 5))
		))

		(if (i32.ne (call $add (i32.const 0x80000000) (i32.const -1)) (i32.const 0x7fffffff)) (then     
			(call $proc_exit (i32.const 6))
		))

		(if (i32.ne (call $add (i32.const 0x80000000) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 7))
		))

		(if (i32.ne (call $add (i32.const 0x3fffffff) (i32.const 1)) (i32.const 0x40000000)) (then     
			(call $proc_exit (i32.const 8))
		))
		
		(if (i32.ne (call $sub (i32.const 1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 9))
		))

		(if (i32.ne (call $sub (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 10))
		))

		(if (i32.ne (call $sub (i32.const -1) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 11))
		))

		(if (i32.ne (call $sub (i32.const 0x7fffffff) (i32.const -1)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 12))
		))

		(if (i32.ne (call $sub (i32.const 0x80000000) (i32.const 1)) (i32.const 0x7fffffff)) (then     
			(call $proc_exit (i32.const 13))
		))

		(if (i32.ne (call $sub (i32.const 0x80000000) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 14))
		))

		(if (i32.ne (call $sub (i32.const 0x3fffffff) (i32.const -1)) (i32.const 0x40000000)) (then     
			(call $proc_exit (i32.const 15))
		))

		(if (i32.ne (call $mul (i32.const 1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 16))
		))

		(if (i32.ne (call $mul (i32.const 1) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 17))
		))

		(if (i32.ne (call $mul (i32.const -1) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 18))
		))

		(if (i32.ne (call $mul (i32.const 0x10000000) (i32.const 4096)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 19))
		))

		(if (i32.ne (call $mul (i32.const 0x80000000) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 20))
		))

		(if (i32.ne (call $mul (i32.const 0x80000000) (i32.const -1)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 21))
		))

		(if (i32.ne (call $mul (i32.const 0x7fffffff) (i32.const -1)) (i32.const 0x80000001)) (then     
			(call $proc_exit (i32.const 22))
		))

		(if (i32.ne (call $mul (i32.const 0x01234567) (i32.const 0x76543210)) (i32.const 0x358e7470)) (then     
			(call $proc_exit (i32.const 23))
		))

		(if (i32.ne (call $mul (i32.const 0x7fffffff) (i32.const 0x7fffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 24))
		))

		;; (assert_trap (invoke "div_s" (i32.const 1) (i32.const 0)) "integer divide by zero")
		;; (assert_trap (invoke "div_s" (i32.const 0) (i32.const 0)) "integer divide by zero")
		;; (assert_trap (invoke "div_s" (i32.const 0x80000000) (i32.const -1)) "integer overflow")
		;; (assert_trap (invoke "div_s" (i32.const 0x80000000) (i32.const 0)) "integer divide by zero")

		(if (i32.ne (call $div_s (i32.const 1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 25))
		))

		(if (i32.ne (call $div_s (i32.const 0) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 26))
		))

		(if (i32.ne (call $div_s (i32.const 0) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 27))
		))

		(if (i32.ne (call $div_s (i32.const -1) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 28))
		))

		(if (i32.ne (call $div_s (i32.const 0x80000000) (i32.const 2)) (i32.const 0xc0000000)) (then     
			(call $proc_exit (i32.const 29))
		))

		(if (i32.ne (call $div_s (i32.const 0x80000001) (i32.const 1000)) (i32.const 0xffdf3b65)) (then     
			(call $proc_exit (i32.const 30))
		))

		(if (i32.ne (call $div_s (i32.const 5) (i32.const 2)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 31))
		))

		(if (i32.ne (call $div_s (i32.const -5) (i32.const 2)) (i32.const -2)) (then     
			(call $proc_exit (i32.const 32))
		))

		(if (i32.ne (call $div_s (i32.const 5) (i32.const -2)) (i32.const -2)) (then     
			(call $proc_exit (i32.const 33))
		))

		(if (i32.ne (call $div_s (i32.const -5) (i32.const -2)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 34))
		))

		(if (i32.ne (call $div_s (i32.const 7) (i32.const 3)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 35))
		))

		(if (i32.ne (call $div_s (i32.const -7) (i32.const 3)) (i32.const -2)) (then     
			(call $proc_exit (i32.const 36))
		))

		(if (i32.ne (call $div_s (i32.const 7) (i32.const -3)) (i32.const -2)) (then     
			(call $proc_exit (i32.const 37))
		))

		(if (i32.ne (call $div_s (i32.const -7) (i32.const -3)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 38))
		))

		(if (i32.ne (call $div_s (i32.const 11) (i32.const 5)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 39))
		))

		(if (i32.ne (call $div_s (i32.const 17) (i32.const 7)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 40))
		))

		;; (assert_trap (invoke "div_u" (i32.const 1) (i32.const 0)) "integer divide by zero")
		;; (assert_trap (invoke "div_u" (i32.const 0) (i32.const 0)) "integer divide by zero")
		(if (i32.ne (call $div_u (i32.const 1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i32.ne (call $div_u (i32.const 0) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 42))
		))

		(if (i32.ne (call $div_u (i32.const -1) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 43))
		))

		(if (i32.ne (call $div_u (i32.const 0x80000000) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 44))
		))

		(if (i32.ne (call $div_u (i32.const 0x80000000) (i32.const 2)) (i32.const 0x40000000)) (then     
			(call $proc_exit (i32.const 45))
		))

		(if (i32.ne (call $div_u (i32.const 0x8ff00ff0) (i32.const 0x10001)) (i32.const 0x8fef)) (then     
			(call $proc_exit (i32.const 46))
		))

		(if (i32.ne (call $div_u (i32.const 0x80000001) (i32.const 1000)) (i32.const 0x20c49b)) (then     
			(call $proc_exit (i32.const 47))
		))

		(if (i32.ne (call $div_u (i32.const 5) (i32.const 2)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 48))
		))

		(if (i32.ne (call $div_u (i32.const -5) (i32.const 2)) (i32.const 0x7ffffffd)) (then     
			(call $proc_exit (i32.const 49))
		))

		(if (i32.ne (call $div_u (i32.const 5) (i32.const -2)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 50))
		))

		(if (i32.ne (call $div_u (i32.const -5) (i32.const -2)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 51))
		))

		(if (i32.ne (call $div_u (i32.const 7) (i32.const 3)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 52))
		))

		(if (i32.ne (call $div_u (i32.const 11) (i32.const 5)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 53))
		))

		(if (i32.ne (call $div_u (i32.const 17) (i32.const 7)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 54))
		))

		;; (assert_trap (invoke "rem_u" (i32.const 1) (i32.const 0)) "integer divide by zero")
		;; (assert_trap (invoke "rem_u" (i32.const 0) (i32.const 0)) "integer divide by zero")
		(if (i32.ne (call $rem_u (i32.const 1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 55))
		))

		(if (i32.ne (call $rem_u (i32.const 0) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 56))
		))

		(if (i32.ne (call $rem_u (i32.const -1) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 57))
		))

		(if (i32.ne (call $rem_u (i32.const 0x80000000) (i32.const -1)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 58))
		))

		(if (i32.ne (call $rem_u (i32.const 0x80000000) (i32.const 2)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 59))
		))

		(if (i32.ne (call $rem_u (i32.const 0x8ff00ff0) (i32.const 0x10001)) (i32.const 0x8001)) (then     
			(call $proc_exit (i32.const 60))
		))

		(if (i32.ne (call $rem_u (i32.const 0x80000001) (i32.const 1000)) (i32.const 649)) (then     
			(call $proc_exit (i32.const 61))
		))

		(if (i32.ne (call $rem_u (i32.const 5) (i32.const 2)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 62))
		))

		(if (i32.ne (call $rem_u (i32.const -5) (i32.const 2)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 63))
		))

		(if (i32.ne (call $rem_u (i32.const 5) (i32.const -2)) (i32.const 5)) (then     
			(call $proc_exit (i32.const 64))
		))

		(if (i32.ne (call $rem_u (i32.const -5) (i32.const -2)) (i32.const -5)) (then     
			(call $proc_exit (i32.const 65))
		))

		(if (i32.ne (call $rem_u (i32.const 7) (i32.const 3)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 66))
		))

		(if (i32.ne (call $rem_u (i32.const 11) (i32.const 5)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 67))
		))

		(if (i32.ne (call $rem_u (i32.const 17) (i32.const 7)) (i32.const 3)) (then     
			(call $proc_exit (i32.const 68))
		))

		(if (i32.ne (call $and (i32.const 1) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 69))
		))

		(if (i32.ne (call $and (i32.const 0) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 70))
		))

		(if (i32.ne (call $and (i32.const 1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 71))
		))

		(if (i32.ne (call $and (i32.const 0) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 72))
		))

		(if (i32.ne (call $and (i32.const 0x7fffffff) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 73))
		))

		(if (i32.ne (call $and (i32.const 0x7fffffff) (i32.const -1)) (i32.const 0x7fffffff)) (then     
			(call $proc_exit (i32.const 74))
		))

		(if (i32.ne (call $and (i32.const 0xf0f0ffff) (i32.const 0xfffff0f0)) (i32.const 0xf0f0f0f0)) (then     
			(call $proc_exit (i32.const 75))
		))

		(if (i32.ne (call $and (i32.const 0xffffffff) (i32.const 0xffffffff)) (i32.const 0xffffffff)) (then     
			(call $proc_exit (i32.const 76))
		))

		(if (i32.ne (call $or (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 77))
		))

		(if (i32.ne (call $or (i32.const 0) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 78))
		))

		(if (i32.ne (call $or (i32.const 1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 79))
		))

		(if (i32.ne (call $or (i32.const 0) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 80))
		))

		(if (i32.ne (call $or (i32.const 0x7fffffff) (i32.const 0x80000000)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 81))
		))

		(if (i32.ne (call $or (i32.const 0x80000000) (i32.const 0)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 82))
		))

		(if (i32.ne (call $or (i32.const 0xf0f0ffff) (i32.const 0xfffff0f0)) (i32.const 0xffffffff)) (then     
			(call $proc_exit (i32.const 83))
		))

		(if (i32.ne (call $or (i32.const 0xffffffff) (i32.const 0xffffffff)) (i32.const 0xffffffff)) (then     
			(call $proc_exit (i32.const 84))
		))

		(if (i32.ne (call $xor (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 85))
		))

		(if (i32.ne (call $xor (i32.const 0) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 86))
		))

		(if (i32.ne (call $xor (i32.const 1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 87))
		))

		(if (i32.ne (call $xor (i32.const 0) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 88))
		))

		(if (i32.ne (call $xor (i32.const 0x7fffffff) (i32.const 0x80000000)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 89))
		))

		(if (i32.ne (call $xor (i32.const 0x80000000) (i32.const 0)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 90))
		))

		(if (i32.ne (call $xor (i32.const -1) (i32.const 0x80000000)) (i32.const 0x7fffffff)) (then     
			(call $proc_exit (i32.const 91))
		))

		(if (i32.ne (call $xor (i32.const -1) (i32.const 0x7fffffff)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 92))
		))

		(if (i32.ne (call $xor (i32.const 0xf0f0ffff) (i32.const 0xfffff0f0)) (i32.const 0x0f0f0f0f)) (then     
			(call $proc_exit (i32.const 93))
		))

		(if (i32.ne (call $xor (i32.const 0xffffffff) (i32.const 0xffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 94))
		))

		(if (i32.ne (call $shl (i32.const 1) (i32.const 1)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 95))
		))

		(if (i32.ne (call $shl (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 96))
		))

		(if (i32.ne (call $shl (i32.const 0x7fffffff) (i32.const 1)) (i32.const 0xfffffffe)) (then     
			(call $proc_exit (i32.const 97))
		))

		(if (i32.ne (call $shl (i32.const 0xffffffff) (i32.const 1)) (i32.const 0xfffffffe)) (then     
			(call $proc_exit (i32.const 98))
		))

		(if (i32.ne (call $shl (i32.const 0x80000000) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 99))
		))

		(if (i32.ne (call $shl (i32.const 0x40000000) (i32.const 1)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 100))
		))

		(if (i32.ne (call $shl (i32.const 1) (i32.const 31)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 101))
		))

		(if (i32.ne (call $shl (i32.const 1) (i32.const 32)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 102))
		))

		(if (i32.ne (call $shl (i32.const 1) (i32.const 33)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 103))
		))

		(if (i32.ne (call $shl (i32.const 1) (i32.const -1)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 104))
		))

		(if (i32.ne (call $shl (i32.const 1) (i32.const 0x7fffffff)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 105))
		))

		(if (i32.ne (call $shr_s (i32.const 1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 106))
		))

		(if (i32.ne (call $shr_s (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 107))
		))

		(if (i32.ne (call $shr_s (i32.const -1) (i32.const 1)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 108))
		))

		(if (i32.ne (call $shr_s (i32.const 0x7fffffff) (i32.const 1)) (i32.const 0x3fffffff)) (then     
			(call $proc_exit (i32.const 109))
		))

		(if (i32.ne (call $shr_s (i32.const 0x80000000) (i32.const 1)) (i32.const 0xc0000000)) (then     
			(call $proc_exit (i32.const 110))
		))

		(if (i32.ne (call $shr_s (i32.const 0x40000000) (i32.const 1)) (i32.const 0x20000000)) (then     
			(call $proc_exit (i32.const 111))
		))

		(if (i32.ne (call $shr_s (i32.const 1) (i32.const 32)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 112))
		))

		(if (i32.ne (call $shr_s (i32.const 1) (i32.const 33)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 113))
		))

		(if (i32.ne (call $shr_s (i32.const 1) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 114))
		))

		(if (i32.ne (call $shr_s (i32.const 1) (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 115))
		))

		(if (i32.ne (call $shr_s (i32.const 1) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 116))
		))

		(if (i32.ne (call $shr_s (i32.const 0x80000000) (i32.const 31)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 117))
		))

		(if (i32.ne (call $shr_s (i32.const -1) (i32.const 32)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 118))
		))

		(if (i32.ne (call $shr_s (i32.const -1) (i32.const 33)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 119))
		))

		(if (i32.ne (call $shr_s (i32.const -1) (i32.const -1)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 120))
		))

		(if (i32.ne (call $shr_s (i32.const -1) (i32.const 0x7fffffff)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 121))
		))

		(if (i32.ne (call $shr_s (i32.const -1) (i32.const 0x80000000)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 122))
		))

		(if (i32.ne (call $shr_u (i32.const 1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 123))
		))

		(if (i32.ne (call $shr_u (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 124))
		))

		(if (i32.ne (call $shr_u (i32.const -1) (i32.const 1)) (i32.const 0x7fffffff)) (then     
			(call $proc_exit (i32.const 125))
		))

		(if (i32.ne (call $shr_u (i32.const 0x7fffffff) (i32.const 1)) (i32.const 0x3fffffff)) (then     
			(call $proc_exit (i32.const 126))
		))

		(if (i32.ne (call $shr_u (i32.const 0x80000000) (i32.const 1)) (i32.const 0x40000000)) (then     
			(call $proc_exit (i32.const 127))
		))

		(if (i32.ne (call $shr_u (i32.const 0x40000000) (i32.const 1)) (i32.const 0x20000000)) (then     
			(call $proc_exit (i32.const 128))
		))

		(if (i32.ne (call $shr_u (i32.const 1) (i32.const 32)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 129))
		))

		(if (i32.ne (call $shr_u (i32.const 1) (i32.const 33)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 130))
		))

		(if (i32.ne (call $shr_u (i32.const 1) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 131))
		))

		(if (i32.ne (call $shr_u (i32.const 1) (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 132))
		))

		(if (i32.ne (call $shr_u (i32.const 1) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 133))
		))

		(if (i32.ne (call $shr_u (i32.const 0x80000000) (i32.const 31)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 134))
		))

		(if (i32.ne (call $shr_u (i32.const -1) (i32.const 32)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 135))
		))

		(if (i32.ne (call $shr_u (i32.const -1) (i32.const 33)) (i32.const 0x7fffffff)) (then     
			(call $proc_exit (i32.const 136))
		))

		(if (i32.ne (call $shr_u (i32.const -1) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 137))
		))

		(if (i32.ne (call $shr_u (i32.const -1) (i32.const 0x7fffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 138))
		))

		(if (i32.ne (call $shr_u (i32.const -1) (i32.const 0x80000000)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 139))
		))

		(if (i32.ne (call $rotl (i32.const 1) (i32.const 1)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 140))
		))

		(if (i32.ne (call $rotl (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 141))
		))

		(if (i32.ne (call $rotl (i32.const -1) (i32.const 1)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 142))
		))

		(if (i32.ne (call $rotl (i32.const 1) (i32.const 32)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 143))
		))

		(if (i32.ne (call $rotl (i32.const 0xabcd9876) (i32.const 1)) (i32.const 0x579b30ed)) (then     
			(call $proc_exit (i32.const 144))
		))

		(if (i32.ne (call $rotl (i32.const 0xfe00dc00) (i32.const 4)) (i32.const 0xe00dc00f)) (then     
			(call $proc_exit (i32.const 145))
		))

		(if (i32.ne (call $rotl (i32.const 0xb0c1d2e3) (i32.const 5)) (i32.const 0x183a5c76)) (then     
			(call $proc_exit (i32.const 146))
		))

		(if (i32.ne (call $rotl (i32.const 0x00008000) (i32.const 37)) (i32.const 0x00100000)) (then     
			(call $proc_exit (i32.const 147))
		))

		(if (i32.ne (call $rotl (i32.const 0xb0c1d2e3) (i32.const 0xff05)) (i32.const 0x183a5c76)) (then     
			(call $proc_exit (i32.const 148))
		))

		(if (i32.ne (call $rotl (i32.const 0x769abcdf) (i32.const 0xffffffed)) (i32.const 0x579beed3)) (then     
			(call $proc_exit (i32.const 149))
		))

		(if (i32.ne (call $rotl (i32.const 0x769abcdf) (i32.const 0x8000000d)) (i32.const 0x579beed3)) (then     
			(call $proc_exit (i32.const 150))
		))

		(if (i32.ne (call $rotl (i32.const 1) (i32.const 31)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 151))
		))

		(if (i32.ne (call $rotl (i32.const 0x80000000) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 152))
		))

		(if (i32.ne (call $rotr (i32.const 1) (i32.const 1)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 153))
		))

		(if (i32.ne (call $rotr (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 154))
		))

		(if (i32.ne (call $rotr (i32.const -1) (i32.const 1)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 155))
		))

		(if (i32.ne (call $rotr (i32.const 1) (i32.const 32)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 156))
		))

		(if (i32.ne (call $rotr (i32.const 0xff00cc00) (i32.const 1)) (i32.const 0x7f806600)) (then     
			(call $proc_exit (i32.const 157))
		))

		(if (i32.ne (call $rotr (i32.const 0x00080000) (i32.const 4)) (i32.const 0x00008000)) (then     
			(call $proc_exit (i32.const 158))
		))

		(if (i32.ne (call $rotr (i32.const 0xb0c1d2e3) (i32.const 5)) (i32.const 0x1d860e97)) (then     
			(call $proc_exit (i32.const 159))
		))

		(if (i32.ne (call $rotr (i32.const 0x00008000) (i32.const 37)) (i32.const 0x00000400)) (then     
			(call $proc_exit (i32.const 160))
		))

		(if (i32.ne (call $rotr (i32.const 0xb0c1d2e3) (i32.const 0xff05)) (i32.const 0x1d860e97)) (then     
			(call $proc_exit (i32.const 161))
		))

		(if (i32.ne (call $rotr (i32.const 0x769abcdf) (i32.const 0xffffffed)) (i32.const 0xe6fbb4d5)) (then     
			(call $proc_exit (i32.const 162))
		))

		(if (i32.ne (call $rotr (i32.const 0x769abcdf) (i32.const 0x8000000d)) (i32.const 0xe6fbb4d5)) (then     
			(call $proc_exit (i32.const 163))
		))

		(if (i32.ne (call $rotr (i32.const 1) (i32.const 31)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 164))
		))

		(if (i32.ne (call $rotr (i32.const 0x80000000) (i32.const 31)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 165))
		))

		(if (i32.ne (call $popcnt (i32.const -1)) (i32.const 32)) (then     
			(call $proc_exit (i32.const 166))
		))

		(if (i32.ne (call $popcnt (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 167))
		))

		(if (i32.ne (call $popcnt (i32.const 0x00008000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 168))
		))

		(if (i32.ne (call $popcnt (i32.const 0x80008000)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 169))
		))

		(if (i32.ne (call $popcnt (i32.const 0x7fffffff)) (i32.const 31)) (then     
			(call $proc_exit (i32.const 170))
		))

		(if (i32.ne (call $popcnt (i32.const 0xAAAAAAAA)) (i32.const 16)) (then     
			(call $proc_exit (i32.const 171))
		))

		(if (i32.ne (call $popcnt (i32.const 0x55555555)) (i32.const 16)) (then     
			(call $proc_exit (i32.const 172))
		))

		(if (i32.ne (call $popcnt (i32.const 0xDEADBEEF)) (i32.const 24)) (then     
			(call $proc_exit (i32.const 173))
		))

		;; (if (i32.ne (call $extend8_s (i32.const 0)) (i32.const 0)) (then     
		;; 	(call $proc_exit (i32.const 174))
		;; ))

		;; (if (i32.ne (call $extend8_s (i32.const 0x7f)) (i32.const 127)) (then     
		;; 	(call $proc_exit (i32.const 175))
		;; ))

		;; (if (i32.ne (call $extend8_s (i32.const 0x80)) (i32.const -128)) (then     
		;; 	(call $proc_exit (i32.const 176))
		;; ))

		;; (if (i32.ne (call $extend8_s (i32.const 0xff)) (i32.const -1)) (then     
		;; 	(call $proc_exit (i32.const 177))
		;; ))

		;; (if (i32.ne (call $extend8_s (i32.const 0x012345_00)) (i32.const 0)) (then     
		;; 	(call $proc_exit (i32.const 178))
		;; ))

		;; (if (i32.ne (call $extend8_s (i32.const 0xfedcba_80)) (i32.const -0x80)) (then     
		;; 	(call $proc_exit (i32.const 179))
		;; ))

		;; (if (i32.ne (call $extend8_s (i32.const -1)) (i32.const -1)) (then     
		;; 	(call $proc_exit (i32.const 180))
		;; ))

		;; (if (i32.ne (call $extend16_s (i32.const 0)) (i32.const 0)) (then     
		;; 	(call $proc_exit (i32.const 181))
		;; ))

		;; (if (i32.ne (call $extend16_s (i32.const 0x7fff)) (i32.const 32767)) (then     
		;; 	(call $proc_exit (i32.const 182))
		;; ))

		;; (if (i32.ne (call $extend16_s (i32.const 0x8000)) (i32.const -32768)) (then     
		;; 	(call $proc_exit (i32.const 183))
		;; ))

		;; (if (i32.ne (call $extend16_s (i32.const 0xffff)) (i32.const -1)) (then     
		;; 	(call $proc_exit (i32.const 184))
		;; ))

		;; (if (i32.ne (call $extend16_s (i32.const 0x0123_0000)) (i32.const 0)) (then     
		;; 	(call $proc_exit (i32.const 185))
		;; ))

		;; (if (i32.ne (call $extend16_s (i32.const 0xfedc_8000)) (i32.const -0x8000)) (then     
		;; 	(call $proc_exit (i32.const 186))
		;; ))

		;; (if (i32.ne (call $extend16_s (i32.const -1)) (i32.const -1)) (then     
		;; 	(call $proc_exit (i32.const 187))
		;; ))

		(if (i32.ne (call $eqz (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 188))
		))

		(if (i32.ne (call $eqz (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 189))
		))

		(if (i32.ne (call $eqz (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 190))
		))

		(if (i32.ne (call $eqz (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 191))
		))

		(if (i32.ne (call $eqz (i32.const 0xffffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 192))
		))

		(if (i32.ne (call $eq (i32.const 0) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 193))
		))

		(if (i32.ne (call $eq (i32.const 1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 194))
		))

		(if (i32.ne (call $eq (i32.const -1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 195))
		))

		(if (i32.ne (call $eq (i32.const 0x80000000) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 196))
		))

		(if (i32.ne (call $eq (i32.const 0x7fffffff) (i32.const 0x7fffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 197))
		))

		(if (i32.ne (call $eq (i32.const -1) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 198))
		))

		(if (i32.ne (call $eq (i32.const 1) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 199))
		))

		(if (i32.ne (call $eq (i32.const 0) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 200))
		))

		(if (i32.ne (call $eq (i32.const 0x80000000) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 201))
		))

		(if (i32.ne (call $eq (i32.const 0) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 202))
		))

		(if (i32.ne (call $eq (i32.const 0x80000000) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 203))
		))

		(if (i32.ne (call $eq (i32.const -1) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 204))
		))

		(if (i32.ne (call $eq (i32.const 0x80000000) (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 205))
		))

		(if (i32.ne (call $eq (i32.const 0x7fffffff) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 206))
		))

		(if (i32.ne (call $ne (i32.const 0) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 207))
		))

		(if (i32.ne (call $ne (i32.const 1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 208))
		))

		(if (i32.ne (call $ne (i32.const -1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 209))
		))

		(if (i32.ne (call $ne (i32.const 0x80000000) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 210))
		))

		(if (i32.ne (call $ne (i32.const 0x7fffffff) (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 211))
		))

		(if (i32.ne (call $ne (i32.const -1) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 212))
		))

		(if (i32.ne (call $ne (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 213))
		))

		(if (i32.ne (call $ne (i32.const 0) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 214))
		))

		(if (i32.ne (call $ne (i32.const 0x80000000) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 215))
		))

		(if (i32.ne (call $ne (i32.const 0) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 216))
		))

		(if (i32.ne (call $ne (i32.const 0x80000000) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 217))
		))

		(if (i32.ne (call $ne (i32.const -1) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 218))
		))

		(if (i32.ne (call $ne (i32.const 0x80000000) (i32.const 0x7fffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ne (i32.const 0x7fffffff) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const 0) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const 1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const -1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const 0x80000000) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const 0x7fffffff) (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const -1) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const 1) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const 0) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const 0x80000000) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const 0) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const 0x80000000) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const -1) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const 0x80000000) (i32.const 0x7fffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_s (i32.const 0x7fffffff) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i32.ne (call $lt_u (i32.const 0) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i32.const 1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i32.const -1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i32.const 0x80000000) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i32.const 0x7fffffff) (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i32.const -1) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i32.const 1) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i32.const 0) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i32.const 0x80000000) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i32.const 0) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i32.const 0x80000000) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i32.const -1) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i32.const 0x80000000) (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $lt_u (i32.const 0x7fffffff) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const 0) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const 1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const -1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const 0x80000000) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const 0x7fffffff) (i32.const 0x7fffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const -1) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const 1) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const 0) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const 0x80000000) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const 0) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const 0x80000000) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const -1) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const 0x80000000) (i32.const 0x7fffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_s (i32.const 0x7fffffff) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const 0) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const 1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const -1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const 0x80000000) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const 0x7fffffff) (i32.const 0x7fffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const -1) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const 1) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const 0) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const 0x80000000) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const 0) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const 0x80000000) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const -1) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const 0x80000000) (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $le_u (i32.const 0x7fffffff) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const 0) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const 1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const -1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const 0x80000000) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const 0x7fffffff) (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const -1) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const 0) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const 0x80000000) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const 0) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const 0x80000000) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const -1) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const 0x80000000) (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_s (i32.const 0x7fffffff) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const 0) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const 1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const -1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const 0x80000000) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const 0x7fffffff) (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const -1) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const 0) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const 0x80000000) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const 0) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const 0x80000000) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const -1) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const 0x80000000) (i32.const 0x7fffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $gt_u (i32.const 0x7fffffff) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const 0) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const 1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const -1) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const 0x80000000) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const 0x7fffffff) (i32.const 0x7fffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const -1) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const 0) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const 0x80000000) (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const 0) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const 0x80000000) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const -1) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const 0x80000000) (i32.const 0x7fffffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_s (i32.const 0x7fffffff) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const 0) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const 1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const -1) (i32.const 1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const 0x80000000) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const 0x7fffffff) (i32.const 0x7fffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const -1) (i32.const -1)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const 1) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const 0) (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const 0x80000000) (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const 0) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const 0x80000000) (i32.const -1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const -1) (i32.const 0x80000000)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const 0x80000000) (i32.const 0x7fffffff)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $ge_u (i32.const 0x7fffffff) (i32.const 0x80000000)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(call $proc_exit (i32.const 0))
	)
)





;; (assert_invalid
;;   (module
;;     (func $type-unary-operand-empty
;;       (i32.eqz) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-unary-operand-empty-in-block
;;       (i32.const 0)
;;       (block (i32.eqz) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-unary-operand-empty-in-loop
;;       (i32.const 0)
;;       (loop (i32.eqz) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-unary-operand-empty-in-if
;;       (i32.const 0) (i32.const 0)
;;       (if (then (i32.eqz) (drop)))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-unary-operand-empty-in-else
;;       (i32.const 0) (i32.const 0)
;;       (if (result i32) (then (i32.const 0)) (else (i32.eqz))) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-unary-operand-empty-in-br
;;       (i32.const 0)
;;       (block (br 0 (i32.eqz)) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-unary-operand-empty-in-br_if
;;       (i32.const 0)
;;       (block (br_if 0 (i32.eqz) (i32.const 1)) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-unary-operand-empty-in-br_table
;;       (i32.const 0)
;;       (block (br_table 0 (i32.eqz)) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-unary-operand-empty-in-return
;;       (return (i32.eqz)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-unary-operand-empty-in-select
;;       (select (i32.eqz) (i32.const 1) (i32.const 2)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-unary-operand-empty-in-call
;;       (call 1 (i32.eqz)) (drop)
;;     )
;;     (func (param i32) (result i32) (local.get 0))
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $f (param i32) (result i32) (local.get 0))
;;     (type $sig (func (param i32) (result i32)))
;;     (table funcref (elem $f))
;;     (func $type-unary-operand-empty-in-call_indirect
;;       (block (result i32)
;;         (call_indirect (type $sig)
;;           (i32.eqz) (i32.const 0)
;;         )
;;         (drop)
;;       )
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-unary-operand-empty-in-local.set
;;       (local i32)
;;       (local.set 0 (i32.eqz)) (local.get 0) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-unary-operand-empty-in-local.tee
;;       (local i32)
;;       (local.tee 0 (i32.eqz)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (global $x (mut i32) (i32.const 0))
;;     (func $type-unary-operand-empty-in-global.set
;;       (global.set $x (i32.eqz)) (global.get $x) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (memory 0)
;;     (func $type-unary-operand-empty-in-memory.grow
;;       (memory.grow (i32.eqz)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (memory 0)
;;     (func $type-unary-operand-empty-in-load
;;       (i32.load (i32.eqz)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (memory 1)
;;     (func $type-unary-operand-empty-in-store
;;       (i32.store (i32.eqz) (i32.const 1))
;;     )
;;   )
;;   "type mismatch"
;; )

;; (assert_invalid
;;   (module
;;     (func $type-binary-1st-operand-empty
;;       (i32.add) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-2nd-operand-empty
;;       (i32.const 0) (i32.add) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-1st-operand-empty-in-block
;;       (i32.const 0) (i32.const 0)
;;       (block (i32.add) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-2nd-operand-empty-in-block
;;       (i32.const 0)
;;       (block (i32.const 0) (i32.add) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-1st-operand-empty-in-loop
;;       (i32.const 0) (i32.const 0)
;;       (loop (i32.add) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-2nd-operand-empty-in-loop
;;       (i32.const 0)
;;       (loop (i32.const 0) (i32.add) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-1st-operand-empty-in-if
;;       (i32.const 0) (i32.const 0) (i32.const 0)
;;       (if (i32.add) (then (drop)))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-2nd-operand-empty-in-if
;;       (i32.const 0) (i32.const 0)
;;       (if (i32.const 0) (then (i32.add)) (else (drop)))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-1st-operand-empty-in-else
;;       (i32.const 0) (i32.const 0) (i32.const 0)
;;       (if (result i32) (then (i32.const 0)) (else (i32.add) (i32.const 0)))
;;       (drop) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-2nd-operand-empty-in-else
;;       (i32.const 0) (i32.const 0)
;;       (if (result i32) (then (i32.const 0)) (else (i32.add)))
;;       (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-1st-operand-empty-in-br
;;       (i32.const 0) (i32.const 0)
;;       (block (br 0 (i32.add)) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-2nd-operand-empty-in-br
;;       (i32.const 0)
;;       (block (br 0 (i32.const 0) (i32.add)) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-1st-operand-empty-in-br_if
;;       (i32.const 0) (i32.const 0)
;;       (block (br_if 0 (i32.add) (i32.const 1)) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-2nd-operand-empty-in-br_if
;;       (i32.const 0)
;;       (block (br_if 0 (i32.const 0) (i32.add) (i32.const 1)) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-1st-operand-empty-in-br_table
;;       (i32.const 0) (i32.const 0)
;;       (block (br_table 0 (i32.add)) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-2nd-operand-empty-in-br_table
;;       (i32.const 0)
;;       (block (br_table 0 (i32.const 0) (i32.add)) (drop))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-1st-operand-empty-in-return
;;       (return (i32.add)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-2nd-operand-empty-in-return
;;       (return (i32.const 0) (i32.add)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-1st-operand-empty-in-select
;;       (select (i32.add) (i32.const 1) (i32.const 2)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-2nd-operand-empty-in-select
;;       (select (i32.const 0) (i32.add) (i32.const 1) (i32.const 2)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-1st-operand-empty-in-call
;;       (call 1 (i32.add)) (drop)
;;     )
;;     (func (param i32 i32) (result i32) (local.get 0))
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-2nd-operand-empty-in-call
;;       (call 1 (i32.const 0) (i32.add)) (drop)
;;     )
;;     (func (param i32 i32) (result i32) (local.get 0))
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $f (param i32) (result i32) (local.get 0))
;;     (type $sig (func (param i32) (result i32)))
;;     (table funcref (elem $f))
;;     (func $type-binary-1st-operand-empty-in-call_indirect
;;       (block (result i32)
;;         (call_indirect (type $sig)
;;           (i32.add) (i32.const 0)
;;         )
;;         (drop)
;;       )
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $f (param i32) (result i32) (local.get 0))
;;     (type $sig (func (param i32) (result i32)))
;;     (table funcref (elem $f))
;;     (func $type-binary-2nd-operand-empty-in-call_indirect
;;       (block (result i32)
;;         (call_indirect (type $sig)
;;           (i32.const 0) (i32.add) (i32.const 0)
;;         )
;;         (drop)
;;       )
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-1st-operand-empty-in-local.set
;;       (local i32)
;;       (local.set 0 (i32.add)) (local.get 0) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-2nd-operand-empty-in-local.set
;;       (local i32)
;;       (local.set 0 (i32.const 0) (i32.add)) (local.get 0) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-1st-operand-empty-in-local.tee
;;       (local i32)
;;       (local.tee 0 (i32.add)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (func $type-binary-2nd-operand-empty-in-local.tee
;;       (local i32)
;;       (local.tee 0 (i32.const 0) (i32.add)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (global $x (mut i32) (i32.const 0))
;;     (func $type-binary-1st-operand-empty-in-global.set
;;       (global.set $x (i32.add)) (global.get $x) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (global $x (mut i32) (i32.const 0))
;;     (func $type-binary-2nd-operand-empty-in-global.set
;;       (global.set $x (i32.const 0) (i32.add)) (global.get $x) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (memory 0)
;;     (func $type-binary-1st-operand-empty-in-memory.grow
;;       (memory.grow (i32.add)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (memory 0)
;;     (func $type-binary-2nd-operand-empty-in-memory.grow
;;       (memory.grow (i32.const 0) (i32.add)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (memory 0)
;;     (func $type-binary-1st-operand-empty-in-load
;;       (i32.load (i32.add)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (memory 0)
;;     (func $type-binary-2nd-operand-empty-in-load
;;       (i32.load (i32.const 0) (i32.add)) (drop)
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (memory 1)
;;     (func $type-binary-1st-operand-empty-in-store
;;       (i32.store (i32.add) (i32.const 1))
;;     )
;;   )
;;   "type mismatch"
;; )
;; (assert_invalid
;;   (module
;;     (memory 1)
;;     (func $type-binary-2nd-operand-empty-in-store
;;       (i32.store (i32.const 1) (i32.add) (i32.const 0))
;;     )
;;   )
;;   "type mismatch"
;; )


;; Type check

;; (assert_invalid (module (func (result i32) (i32.add (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.and (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.div_s (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.div_u (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.mul (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.or (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.rem_u (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.rotl (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.rotr (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.shl (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.shr_s (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.shr_u (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.sub (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.xor (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.eqz (i64.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.clz (i64.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.ctz (i64.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.popcnt (i64.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.eq (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.ge_s (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.ge_u (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.gt_s (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.gt_u (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.le_s (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.le_u (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.lt_s (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.lt_u (i64.const 0) (f32.const 0)))) "type mismatch")
;; (assert_invalid (module (func (result i32) (i32.ne (i64.const 0) (f32.const 0)))) "type mismatch")
