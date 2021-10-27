(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)

	(func $i32.wrap_i64 (export "i32.wrap_i64") (param $x i64) (result i32) (i32.wrap_i64 (local.get $x)))
	(func $i32.trunc_f32_s (export "i32.trunc_f32_s") (param $x f32) (result i32) (i32.trunc_f32_s (local.get $x)))
	(func $i32.trunc_f32_u (export "i32.trunc_f32_u") (param $x f32) (result i32) (i32.trunc_f32_u (local.get $x)))
	(func $i64.trunc_f32_s (export "i64.trunc_f32_s") (param $x f32) (result i64) (i64.trunc_f32_s (local.get $x)))
	(func $i64.trunc_f64_s (export "i64.trunc_f64_s") (param $x f64) (result i64) (i64.trunc_f64_s (local.get $x)))
	;; (func $i32.trunc_sat_f32_s (export "i32.trunc_sat_f32_s") (param $x f32) (result i32) (i32.trunc_sat_f32_s (local.get $x)))
	;; (func $i32.trunc_sat_f32_u (export "i32.trunc_sat_f32_u") (param $x f32) (result i32) (i32.trunc_sat_f32_u (local.get $x)))
	;; (func $i32.trunc_sat_f64_s (export "i32.trunc_sat_f64_s") (param $x f64) (result i32) (i32.trunc_sat_f64_s (local.get $x)))
	;; (func $i32.trunc_sat_f64_u (export "i32.trunc_sat_f64_u") (param $x f64) (result i32) (i32.trunc_sat_f64_u (local.get $x)))
	;; (func $i64.trunc_sat_f32_s (export "i64.trunc_sat_f32_s") (param $x f32) (result i64) (i64.trunc_sat_f32_s (local.get $x)))
	;; (func $i64.trunc_sat_f32_u (export "i64.trunc_sat_f32_u") (param $x f32) (result i64) (i64.trunc_sat_f32_u (local.get $x)))
	;; (func $i64.trunc_sat_f64_s (export "i64.trunc_sat_f64_s") (param $x f64) (result i64) (i64.trunc_sat_f64_s (local.get $x)))
	;; (func $i64.trunc_sat_f64_u (export "i64.trunc_sat_f64_u") (param $x f64) (result i64) (i64.trunc_sat_f64_u (local.get $x)))
	(func $f32.convert_i32_s (export "f32.convert_i32_s") (param $x i32) (result f32) (f32.convert_i32_s (local.get $x)))
	(func $f32.convert_i64_s (export "f32.convert_i64_s") (param $x i64) (result f32) (f32.convert_i64_s (local.get $x)))
	(func $f64.convert_i32_s (export "f64.convert_i32_s") (param $x i32) (result f64) (f64.convert_i32_s (local.get $x)))
	(func $f64.convert_i64_s (export "f64.convert_i64_s") (param $x i64) (result f64) (f64.convert_i64_s (local.get $x)))
	(func $f32.convert_i32_u (export "f32.convert_i32_u") (param $x i32) (result f32) (f32.convert_i32_u (local.get $x)))
	(func $f32.convert_i64_u (export "f32.convert_i64_u") (param $x i64) (result f32) (f32.convert_i64_u (local.get $x)))
	(func $f64.convert_i32_u (export "f64.convert_i32_u") (param $x i32) (result f64) (f64.convert_i32_u (local.get $x)))
	(func $f64.convert_i64_u (export "f64.convert_i64_u") (param $x i64) (result f64) (f64.convert_i64_u (local.get $x)))
	(func $f64.promote_f32 (export "f64.promote_f32") (param $x f32) (result f64) (f64.promote_f32 (local.get $x)))
	(func $f32.demote_f64 (export "f32.demote_f64") (param $x f64) (result f32) (f32.demote_f64 (local.get $x)))
	(func $f32.reinterpret_i32 (export "f32.reinterpret_i32") (param $x i32) (result f32) (f32.reinterpret_i32 (local.get $x)))
	(func $f64.reinterpret_i64 (export "f64.reinterpret_i64") (param $x i64) (result f64) (f64.reinterpret_i64 (local.get $x)))
	(func $i32.reinterpret_f32 (export "i32.reinterpret_f32") (param $x f32) (result i32) (i32.reinterpret_f32 (local.get $x)))
	(func $i64.reinterpret_f64 (export "i64.reinterpret_f64") (param $x f64) (result i64) (i64.reinterpret_f64 (local.get $x)))


	(func $_start (export "_start")
		(if (i32.ne (call $i32.wrap_i64 (i64.const -100000)) (i32.const -100000)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $i32.wrap_i64 (i64.const -100000)) (i32.const -100000)) (then     
			(call $proc_exit (i32.const 2))
		))

		(if (i32.ne (call $i32.wrap_i64 (i64.const 0x80000000)) (i32.const 0x80000000)) (then     
			(call $proc_exit (i32.const 3))
		))

		(if (i32.ne (call $i32.wrap_i64 (i64.const 0xffffffff7fffffff)) (i32.const 0x7fffffff)) (then     
			(call $proc_exit (i32.const 4))
		))

		(if (i32.ne (call $i32.wrap_i64 (i64.const 0xffffffff00000000)) (i32.const 0x00000000)) (then     
			(call $proc_exit (i32.const 5))
		))

		(if (i32.ne (call $i32.wrap_i64 (i64.const 0xfffffffeffffffff)) (i32.const 0xffffffff)) (then     
			(call $proc_exit (i32.const 6))
		))

		(if (i32.ne (call $i32.wrap_i64 (i64.const 0xffffffff00000001)) (i32.const 0x00000001)) (then     
			(call $proc_exit (i32.const 7))
		))

		(if (i32.ne (call $i32.wrap_i64 (i64.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 8))
		))

		(if (i32.ne (call $i32.wrap_i64 (i64.const 1311768467463790320)) (i32.const 0x9abcdef0)) (then     
			(call $proc_exit (i32.const 9))
		))

		(if (i32.ne (call $i32.wrap_i64 (i64.const 0x00000000ffffffff)) (i32.const 0xffffffff)) (then     
			(call $proc_exit (i32.const 10))
		))

		(if (i32.ne (call $i32.wrap_i64 (i64.const 0x0000000100000000)) (i32.const 0x00000000)) (then     
			(call $proc_exit (i32.const 11))
		))

		(if (i32.ne (call $i32.wrap_i64 (i64.const 0x0000000100000001)) (i32.const 0x00000001)) (then     
			(call $proc_exit (i32.const 12))
		))

		(if (i32.ne (call $i32.trunc_f32_s (f32.const 0.0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 13))
		))

		(if (i32.ne (call $i32.trunc_f32_s (f32.const -0.0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 14))
		))

		(if (i32.ne (call $i32.trunc_f32_s (f32.const 0x1p-149)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 15))
		))

		(if (i32.ne (call $i32.trunc_f32_s (f32.const 1.0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 16))
		))

		(if (i32.ne (call $i32.trunc_f32_s (f32.const 0x1.19999ap+0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 17))
		))

		(if (i32.ne (call $i32.trunc_f32_s (f32.const 1.5)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 18))
		))

		(if (i32.ne (call $i32.trunc_f32_s (f32.const -1.0)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 19))
		))

		(if (i32.ne (call $i32.trunc_f32_s (f32.const -0x1.19999ap+0)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 20))
		))

		(if (i32.ne (call $i32.trunc_f32_s (f32.const -1.5)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 21))
		))

		(if (i32.ne (call $i32.trunc_f32_s (f32.const -1.9)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 22))
		))

		(if (i32.ne (call $i32.trunc_f32_s (f32.const -2.0)) (i32.const -2)) (then     
			(call $proc_exit (i32.const 23))
		))

		(if (i32.ne (call $i32.trunc_f32_s (f32.const 2147483520.0)) (i32.const 2147483520)) (then     
			(call $proc_exit (i32.const 24))
		))

		(if (i32.ne (call $i32.trunc_f32_s (f32.const -2147483648.0)) (i32.const -2147483648)) (then     
			(call $proc_exit (i32.const 25))
		))

		;; (assert_trap (invoke "i32.trunc_f32_s" (f32.const 2147483648.0)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f32_s" (f32.const -2147483904.0)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f32_s" (f32.const inf)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f32_s" (f32.const -inf)) "integer overflow")
		;; (assert_trap (invoke "i32.trunc_f32_s" (f32.const nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i32.trunc_f32_s" (f32.const nan:0x200000)) "invalid conversion to integer")
		;; (assert_trap (invoke "i32.trunc_f32_s" (f32.const -nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i32.trunc_f32_s" (f32.const -nan:0x200000)) "invalid conversion to integer")

		(if (i64.ne (call $i64.trunc_f32_s (f32.const 0.0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 26))
		))

		(if (i64.ne (call $i64.trunc_f32_s (f32.const -0.0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 27))
		))

		(if (i64.ne (call $i64.trunc_f32_s (f32.const 0x1p-149)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 28))
		))

		(if (i64.ne (call $i64.trunc_f32_s (f32.const -0x1p-149)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 29))
		))

		(if (i64.ne (call $i64.trunc_f32_s (f32.const 1.0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 30))
		))

		(if (i64.ne (call $i64.trunc_f32_s (f32.const 0x1.19999ap+0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 31))
		))

		(if (i64.ne (call $i64.trunc_f32_s (f32.const 1.5)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 32))
		))

		(if (i64.ne (call $i64.trunc_f32_s (f32.const -1.0)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 33))
		))

		(if (i64.ne (call $i64.trunc_f32_s (f32.const -0x1.19999ap+0)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 34))
		))

		(if (i64.ne (call $i64.trunc_f32_s (f32.const -1.5)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 35))
		))

		(if (i64.ne (call $i64.trunc_f32_s (f32.const -1.9)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 36))
		))

		(if (i64.ne (call $i64.trunc_f32_s (f32.const -2.0)) (i64.const -2)) (then     
			(call $proc_exit (i32.const 37))
		))

		;; 0x1.00000p+32 -> 1 0000 0000
		(if (i64.ne (call $i64.trunc_f32_s (f32.const 4294967296)) (i64.const 4294967296)) (then     
			(call $proc_exit (i32.const 38))
		))

		;; -0x1.00000p+32 -> ffff ffff 0000 0000
		(if (i64.ne (call $i64.trunc_f32_s (f32.const -4294967296)) (i64.const -4294967296)) (then     
			(call $proc_exit (i32.const 39))
		))

		(if (i64.ne (call $i64.trunc_f32_s (f32.const 9223371487098961920.0)) (i64.const 9223371487098961920)) (then     
			(call $proc_exit (i32.const 40))
		))

		(if (i64.ne (call $i64.trunc_f32_s (f32.const -9223372036854775808.0)) (i64.const -9223372036854775808)) (then     
			(call $proc_exit (i32.const 41))
		))

		;; (assert_trap (invoke "i64.trunc_f32_s" (f32.const 9223372036854775808.0)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f32_s" (f32.const -9223373136366403584.0)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f32_s" (f32.const inf)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f32_s" (f32.const -inf)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f32_s" (f32.const nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i64.trunc_f32_s" (f32.const nan:0x200000)) "invalid conversion to integer")
		;; (assert_trap (invoke "i64.trunc_f32_s" (f32.const -nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i64.trunc_f32_s" (f32.const -nan:0x200000)) "invalid conversion to integer")

		(if (i64.ne (call $i64.trunc_f64_s (f64.const 0.0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 42))
		))

		(if (i64.ne (call $i64.trunc_f64_s (f64.const -0.0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 43))
		))

		(if (i64.ne (call $i64.trunc_f64_s (f64.const 0x0.0000000000001p-1022)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 44))
		))

		(if (i64.ne (call $i64.trunc_f64_s (f64.const -0x0.0000000000001p-1022)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 45))
		))

		(if (i64.ne (call $i64.trunc_f64_s (f64.const 1.0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 46))
		))

		(if (i64.ne (call $i64.trunc_f64_s (f64.const 0x1.199999999999ap+0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 47))
		))

		(if (i64.ne (call $i64.trunc_f64_s (f64.const 1.5)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 48))
		))

		(if (i64.ne (call $i64.trunc_f64_s (f64.const -1.0)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 49))
		))

		(if (i64.ne (call $i64.trunc_f64_s (f64.const -0x1.199999999999ap+0)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 50))
		))

		(if (i64.ne (call $i64.trunc_f64_s (f64.const -1.5)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 51))
		))

		(if (i64.ne (call $i64.trunc_f64_s (f64.const -1.9)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 52))
		))

		(if (i64.ne (call $i64.trunc_f64_s (f64.const -2.0)) (i64.const -2)) (then     
			(call $proc_exit (i32.const 53))
		))

		;; 0x1.00000p+32 -> 1 0000 0000
		(if (i64.ne (call $i64.trunc_f64_s (f64.const 4294967296)) (i64.const 4294967296)) (then     
			(call $proc_exit (i32.const 54))
		))
		
		;; -0x1.00000p+32 -> ffff ffff 0000 0000
		(if (i64.ne (call $i64.trunc_f64_s (f64.const -4294967296)) (i64.const -4294967296)) (then     
			(call $proc_exit (i32.const 55))
		))
		
		(if (i64.ne (call $i64.trunc_f64_s (f64.const 9223372036854774784.0)) (i64.const 9223372036854774784)) (then     
			(call $proc_exit (i32.const 56))
		))

		(if (i64.ne (call $i64.trunc_f64_s (f64.const -9223372036854775808.0)) (i64.const -9223372036854775808)) (then     
			(call $proc_exit (i32.const 57))
		))

		;; (assert_trap (invoke "i64.trunc_f64_s" (f64.const 9223372036854775808.0)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f64_s" (f64.const -9223372036854777856.0)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f64_s" (f64.const inf)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f64_s" (f64.const -inf)) "integer overflow")
		;; (assert_trap (invoke "i64.trunc_f64_s" (f64.const nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i64.trunc_f64_s" (f64.const nan:0x4000000000000)) "invalid conversion to integer")
		;; (assert_trap (invoke "i64.trunc_f64_s" (f64.const -nan)) "invalid conversion to integer")
		;; (assert_trap (invoke "i64.trunc_f64_s" (f64.const -nan:0x4000000000000)) "invalid conversion to integer")
		
		(if (f32.ne (call $f32.convert_i32_s (i32.const 1)) (f32.const 1.0)) (then     
			(call $proc_exit (i32.const 58))
		))

		(if (f32.ne (call $f32.convert_i32_s (i32.const -1)) (f32.const -1.0)) (then     
			(call $proc_exit (i32.const 59))
		))

		(if (f32.ne (call $f32.convert_i32_s (i32.const 0)) (f32.const 0.0)) (then     
			(call $proc_exit (i32.const 60))
		))

		(if (f32.ne (call $f32.convert_i32_s (i32.const 2147483647)) (f32.const 2147483648)) (then     
			(call $proc_exit (i32.const 61))
		))

		(if (f32.ne (call $f32.convert_i32_s (i32.const -2147483648)) (f32.const -2147483648)) (then     
			(call $proc_exit (i32.const 62))
		))

		(if (f32.ne (call $f32.convert_i32_s (i32.const 1234567890)) (f32.const 0x1.26580cp+30)) (then     
			(call $proc_exit (i32.const 63))
		))

		;; Test rounding directions.
		(if (f32.ne (call $f32.convert_i32_s (i32.const 16777217)) (f32.const 16777216.0)) (then     
			(call $proc_exit (i32.const 64))
		))

		(if (f32.ne (call $f32.convert_i32_s (i32.const -16777217)) (f32.const -16777216.0)) (then     
			(call $proc_exit (i32.const 65))
		))

		(if (f32.ne (call $f32.convert_i32_s (i32.const 16777219)) (f32.const 16777220.0)) (then     
			(call $proc_exit (i32.const 66))
		))

		(if (f32.ne (call $f32.convert_i32_s (i32.const -16777219)) (f32.const -16777220.0)) (then     
			(call $proc_exit (i32.const 67))
		))

		(if (f32.ne (call $f32.convert_i64_s (i64.const 1)) (f32.const 1.0)) (then     
			(call $proc_exit (i32.const 68))
		))

		(if (f32.ne (call $f32.convert_i64_s (i64.const -1)) (f32.const -1.0)) (then     
			(call $proc_exit (i32.const 69))
		))

		(if (f32.ne (call $f32.convert_i64_s (i64.const 0)) (f32.const 0.0)) (then     
			(call $proc_exit (i32.const 70))
		))

		(if (f32.ne (call $f32.convert_i64_s (i64.const 9223372036854775807)) (f32.const 9223372036854775807)) (then     
			(call $proc_exit (i32.const 71))
		))

		(if (f32.ne (call $f32.convert_i64_s (i64.const -9223372036854775808)) (f32.const -9223372036854775808)) (then     
			(call $proc_exit (i32.const 72))
		))

		;; PI
		(if (f32.ne (call $f32.convert_i64_s (i64.const 314159265358979)) (f32.const 0x1.1db9e8p+48))  (then     
			(call $proc_exit (i32.const 73))
		))
		
		;; Test rounding directions.
		(if (f32.ne (call $f32.convert_i64_s (i64.const 16777217)) (f32.const 16777216.0)) (then     
			(call $proc_exit (i32.const 74))
		))

		(if (f32.ne (call $f32.convert_i64_s (i64.const -16777217)) (f32.const -16777216.0)) (then     
			(call $proc_exit (i32.const 75))
		))

		(if (f32.ne (call $f32.convert_i64_s (i64.const 16777219)) (f32.const 16777220.0)) (then     
			(call $proc_exit (i32.const 76))
		))

		(if (f32.ne (call $f32.convert_i64_s (i64.const -16777219)) (f32.const -16777220.0)) (then     
			(call $proc_exit (i32.const 77))
		))

		(if (f32.ne (call $f32.convert_i64_s (i64.const 0x7fffff4000000001)) (f32.const 0x1.fffffep+62)) (then     
			(call $proc_exit (i32.const 78))
		))

		(if (f32.ne (call $f32.convert_i64_s (i64.const 0x8000004000000001)) (f32.const -0x1.fffffep+62)) (then     
			(call $proc_exit (i32.const 79))
		))

		(if (f32.ne (call $f32.convert_i64_s (i64.const 0x0020000020000001)) (f32.const 0x1.000002p+53)) (then     
			(call $proc_exit (i32.const 80))
		))

		(if (f32.ne (call $f32.convert_i64_s (i64.const 0xffdfffffdfffffff)) (f32.const -0x1.000002p+53)) (then     
			(call $proc_exit (i32.const 81))
		))

		(if (f64.ne (call $f64.convert_i32_s (i32.const 1)) (f64.const 1.0)) (then     
			(call $proc_exit (i32.const 82))
		))

		(if (f64.ne (call $f64.convert_i32_s (i32.const -1)) (f64.const -1.0)) (then     
			(call $proc_exit (i32.const 83))
		))

		(if (f64.ne (call $f64.convert_i32_s (i32.const 0)) (f64.const 0.0)) (then     
			(call $proc_exit (i32.const 84))
		))

		(if (f64.ne (call $f64.convert_i32_s (i32.const 2147483647)) (f64.const 2147483647)) (then     
			(call $proc_exit (i32.const 85))
		))

		(if (f64.ne (call $f64.convert_i32_s (i32.const -2147483648)) (f64.const -2147483648)) (then     
			(call $proc_exit (i32.const 86))
		))

		(if (f64.ne (call $f64.convert_i32_s (i32.const 987654321)) (f64.const 987654321)) (then     
			(call $proc_exit (i32.const 87))
		))

		(if (f64.ne (call $f64.convert_i64_s (i64.const 1)) (f64.const 1.0)) (then     
			(call $proc_exit (i32.const 88))
		))

		(if (f64.ne (call $f64.convert_i64_s (i64.const -1)) (f64.const -1.0)) (then     
			(call $proc_exit (i32.const 89))
		))

		(if (f64.ne (call $f64.convert_i64_s (i64.const 0)) (f64.const 0.0)) (then     
			(call $proc_exit (i32.const 90))
		))

		(if (f64.ne (call $f64.convert_i64_s (i64.const 9223372036854775807)) (f64.const 9223372036854775807)) (then     
			(call $proc_exit (i32.const 91))
		))

		(if (f64.ne (call $f64.convert_i64_s (i64.const -9223372036854775808)) (f64.const -9223372036854775808)) (then     
			(call $proc_exit (i32.const 92))
		))

		;; Feigenbaum
		(if (f64.ne (call $f64.convert_i64_s (i64.const 4669201609102990)) (f64.const 4669201609102990))  (then     
			(call $proc_exit (i32.const 93))
		))
		
		;; Test rounding directions.
		(if (f64.ne (call $f64.convert_i64_s (i64.const 9007199254740993)) (f64.const 9007199254740992)) (then     
			(call $proc_exit (i32.const 94))
		))

		(if (f64.ne (call $f64.convert_i64_s (i64.const -9007199254740993)) (f64.const -9007199254740992)) (then     
			(call $proc_exit (i32.const 95))
		))

		(if (f64.ne (call $f64.convert_i64_s (i64.const 9007199254740995)) (f64.const 9007199254740996)) (then     
			(call $proc_exit (i32.const 96))
		))

		(if (f64.ne (call $f64.convert_i64_s (i64.const -9007199254740995)) (f64.const -9007199254740996)) (then     
			(call $proc_exit (i32.const 97))
		))

		(if (f32.ne (call $f32.convert_i32_u (i32.const 1)) (f32.const 1.0)) (then     
			(call $proc_exit (i32.const 98))
		))

		(if (f32.ne (call $f32.convert_i32_u (i32.const 0)) (f32.const 0.0)) (then     
			(call $proc_exit (i32.const 99))
		))

		(if (f32.ne (call $f32.convert_i32_u (i32.const 2147483647)) (f32.const 2147483648)) (then     
			(call $proc_exit (i32.const 100))
		))

		(if (f32.ne (call $f32.convert_i32_u (i32.const -2147483648)) (f32.const 2147483648)) (then     
			(call $proc_exit (i32.const 101))
		))

		(if (f32.ne (call $f32.convert_i32_u (i32.const 0x12345678)) (f32.const 0x1.234568p+28)) (then     
			(call $proc_exit (i32.const 102))
		))

		(if (f32.ne (call $f32.convert_i32_u (i32.const 0xffffffff)) (f32.const 4294967296.0)) (then     
			(call $proc_exit (i32.const 103))
		))

		(if (f32.ne (call $f32.convert_i32_u (i32.const 0x80000080)) (f32.const 0x1.000000p+31)) (then     
			(call $proc_exit (i32.const 104))
		))

		(if (f32.ne (call $f32.convert_i32_u (i32.const 0x80000081)) (f32.const 0x1.000002p+31)) (then     
			(call $proc_exit (i32.const 105))
		))

		(if (f32.ne (call $f32.convert_i32_u (i32.const 0x80000082)) (f32.const 0x1.000002p+31)) (then     
			(call $proc_exit (i32.const 106))
		))

		(if (f32.ne (call $f32.convert_i32_u (i32.const 0xfffffe80)) (f32.const 0x1.fffffcp+31)) (then     
			(call $proc_exit (i32.const 107))
		))

		(if (f32.ne (call $f32.convert_i32_u (i32.const 0xfffffe81)) (f32.const 0x1.fffffep+31)) (then     
			(call $proc_exit (i32.const 108))
		))

		(if (f32.ne (call $f32.convert_i32_u (i32.const 0xfffffe82)) (f32.const 0x1.fffffep+31)) (then     
			(call $proc_exit (i32.const 109))
		))

		;; Test rounding directions.
		(if (f32.ne (call $f32.convert_i32_u (i32.const 16777217)) (f32.const 16777216.0)) (then     
			(call $proc_exit (i32.const 110))
		))

		(if (f32.ne (call $f32.convert_i32_u (i32.const 16777219)) (f32.const 16777220.0)) (then     
			(call $proc_exit (i32.const 111))
		))


		(if (f32.ne (call $f32.convert_i64_u (i64.const 1)) (f32.const 1.0)) (then     
			(call $proc_exit (i32.const 112))
		))

		(if (f32.ne (call $f32.convert_i64_u (i64.const 0)) (f32.const 0.0)) (then     
			(call $proc_exit (i32.const 113))
		))

		(if (f32.ne (call $f32.convert_i64_u (i64.const 9223372036854775807)) (f32.const 9223372036854775807)) (then     
			(call $proc_exit (i32.const 114))
		))

		(if (f32.ne (call $f32.convert_i64_u (i64.const -9223372036854775808)) (f32.const 9223372036854775808)) (then     
			(call $proc_exit (i32.const 115))
		))

		(if (f32.ne (call $f32.convert_i64_u (i64.const 0xffffffffffffffff)) (f32.const 18446744073709551616.0)) (then     
			(call $proc_exit (i32.const 116))
		))

		;; Test rounding directions.
		(if (f32.ne (call $f32.convert_i64_u (i64.const 16777217)) (f32.const 16777216.0)) (then     
			(call $proc_exit (i32.const 117))
		))

		(if (f32.ne (call $f32.convert_i64_u (i64.const 16777219)) (f32.const 16777220.0)) (then     
			(call $proc_exit (i32.const 118))
		))

		(if (f32.ne (call $f32.convert_i64_u (i64.const 0x0020000020000001)) (f32.const 0x1.000002p+53)) (then     
			(call $proc_exit (i32.const 119))
		))

		(if (f32.ne (call $f32.convert_i64_u (i64.const 0x7fffffbfffffffff)) (f32.const 0x1.fffffep+62)) (then     
			(call $proc_exit (i32.const 120))
		))

		(if (f32.ne (call $f32.convert_i64_u (i64.const 0x8000008000000001)) (f32.const 0x1.000002p+63)) (then     
			(call $proc_exit (i32.const 121))
		))

		(if (f32.ne (call $f32.convert_i64_u (i64.const 0xfffffe8000000001)) (f32.const 0x1.fffffep+63)) (then     
			(call $proc_exit (i32.const 122))
		))

		(if (f64.ne (call $f64.convert_i32_u (i32.const 1)) (f64.const 1.0))(then     
			(call $proc_exit (i32.const 123))
		))

		(if (f64.ne (call $f64.convert_i32_u (i32.const 0)) (f64.const 0.0))(then     
			(call $proc_exit (i32.const 124))
		))

		(if (f64.ne (call $f64.convert_i32_u (i32.const 2147483647)) (f64.const 2147483647))(then     
			(call $proc_exit (i32.const 125))
		))

		(if (f64.ne (call $f64.convert_i32_u (i32.const -2147483648)) (f64.const 2147483648))(then     
			(call $proc_exit (i32.const 126))
		))

		(if (f64.ne (call $f64.convert_i32_u (i32.const 0xffffffff)) (f64.const 4294967295.0))(then     
			(call $proc_exit (i32.const 127))
		))
		
		(if (f64.ne (call $f64.convert_i64_u (i64.const 1)) (f64.const 1.0))(then     
			(call $proc_exit (i32.const 128))
		))

		(if (f64.ne (call $f64.convert_i64_u (i64.const 0)) (f64.const 0.0))(then     
			(call $proc_exit (i32.const 129))
		))

		(if (f64.ne (call $f64.convert_i64_u (i64.const 9223372036854775807)) (f64.const 9223372036854775807))(then     
			(call $proc_exit (i32.const 130))
		))

		(if (f64.ne (call $f64.convert_i64_u (i64.const -9223372036854775808)) (f64.const 9223372036854775808))(then     
			(call $proc_exit (i32.const 131))
		))

		(if (f64.ne (call $f64.convert_i64_u (i64.const 0xffffffffffffffff)) (f64.const 18446744073709551616.0))(then     
			(call $proc_exit (i32.const 132))
		))

		(if (f64.ne (call $f64.convert_i64_u (i64.const 0x8000000000000400)) (f64.const 0x1.0000000000000p+63))(then     
			(call $proc_exit (i32.const 133))
		))

		(if (f64.ne (call $f64.convert_i64_u (i64.const 0x8000000000000401)) (f64.const 0x1.0000000000001p+63))(then     
			(call $proc_exit (i32.const 134))
		))

		(if (f64.ne (call $f64.convert_i64_u (i64.const 0x8000000000000402)) (f64.const 0x1.0000000000001p+63))(then     
			(call $proc_exit (i32.const 135))
		))

		(if (f64.ne (call $f64.convert_i64_u (i64.const 0xfffffffffffff400)) (f64.const 0x1.ffffffffffffep+63))(then     
			(call $proc_exit (i32.const 136))
		))

		(if (f64.ne (call $f64.convert_i64_u (i64.const 0xfffffffffffff401)) (f64.const 0x1.fffffffffffffp+63))(then     
			(call $proc_exit (i32.const 137))
		))

		(if (f64.ne (call $f64.convert_i64_u (i64.const 0xfffffffffffff402)) (f64.const 0x1.fffffffffffffp+63))(then     
			(call $proc_exit (i32.const 138))
		))

		;; Test rounding directions.
		(if (f64.ne (call $f64.convert_i64_u (i64.const 9007199254740993)) (f64.const 9007199254740992))(then     
			(call $proc_exit (i32.const 139))
		))

		(if (f64.ne (call $f64.convert_i64_u (i64.const 9007199254740995)) (f64.const 9007199254740996))(then     
			(call $proc_exit (i32.const 140))
		))

		(if (f64.ne (call $f64.promote_f32 (f32.const 0.0)) (f64.const 0.0))(then     
			(call $proc_exit (i32.const 141))
		))

		(if (f64.ne (call $f64.promote_f32 (f32.const -0.0)) (f64.const -0.0))(then     
			(call $proc_exit (i32.const 142))
		))

		(if (f64.ne (call $f64.promote_f32 (f32.const 0x1p-149)) (f64.const 0x1p-149))(then     
			(call $proc_exit (i32.const 143))
		))

		(if (f64.ne (call $f64.promote_f32 (f32.const -0x1p-149)) (f64.const -0x1p-149))(then     
			(call $proc_exit (i32.const 144))
		))

		(if (f64.ne (call $f64.promote_f32 (f32.const 1.0)) (f64.const 1.0))(then     
			(call $proc_exit (i32.const 145))
		))

		(if (f64.ne (call $f64.promote_f32 (f32.const -1.0)) (f64.const -1.0))(then     
			(call $proc_exit (i32.const 146))
		))

		(if (f64.ne (call $f64.promote_f32 (f32.const -0x1.fffffep+127)) (f64.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 147))
		))

		(if (f64.ne (call $f64.promote_f32 (f32.const 0x1.fffffep+127)) (f64.const 0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 148))
		))

		;; Generated randomly by picking a random int and reinterpret it to float.
		(if (f64.ne (call $f64.promote_f32 (f32.const 0x1p-119)) (f64.const 0x1p-119))(then     
			(call $proc_exit (i32.const 149))
		))

		;; Generated randomly by picking a random float.
		(if (f64.ne (call $f64.promote_f32 (f32.const 0x1.8f867ep+125)) (f64.const 6.6382536710104395e+37))(then     
			(call $proc_exit (i32.const 150))
		))

		(if (f64.ne (call $f64.promote_f32 (f32.const inf)) (f64.const inf))(then     
			(call $proc_exit (i32.const 151))
		))

		(if (f64.ne (call $f64.promote_f32 (f32.const -inf)) (f64.const -inf))(then     
			(call $proc_exit (i32.const 152))
		))

		;; (if (f64.ne (call $f64.promote_f32 (f32.const nan)) (f64.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 153))
		;; ))

		;; (if (f64.ne (call $f64.promote_f32 (f32.const nan:0x200000)) (f64.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 154))
		;; ))

		;; (if (f64.ne (call $f64.promote_f32 (f32.const -nan)) (f64.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 155))
		;; ))

		;; (if (f64.ne (call $f64.promote_f32 (f32.const -nan:0x200000)) (f64.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 156))
		;; ))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0.0)) (f32.const 0.0))(then     
			(call $proc_exit (i32.const 157))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0.0)) (f32.const -0.0))(then     
			(call $proc_exit (i32.const 158))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x0.0000000000001p-1022)) (f32.const 0.0))(then     
			(call $proc_exit (i32.const 159))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x0.0000000000001p-1022)) (f32.const -0.0))(then     
			(call $proc_exit (i32.const 160))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 1.0)) (f32.const 1.0))(then     
			(call $proc_exit (i32.const 161))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -1.0)) (f32.const -1.0))(then     
			(call $proc_exit (i32.const 162))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.fffffe0000000p-127)) (f32.const 0x1p-126))(then     
			(call $proc_exit (i32.const 163))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x1.fffffe0000000p-127)) (f32.const -0x1p-126))(then     
			(call $proc_exit (i32.const 164))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.fffffdfffffffp-127)) (f32.const 0x1.fffffcp-127))(then     
			(call $proc_exit (i32.const 165))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x1.fffffdfffffffp-127)) (f32.const -0x1.fffffcp-127))(then     
			(call $proc_exit (i32.const 166))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1p-149)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 167))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x1p-149)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 168))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.fffffd0000000p+127)) (f32.const 0x1.fffffcp+127))(then     
			(call $proc_exit (i32.const 169))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x1.fffffd0000000p+127)) (f32.const -0x1.fffffcp+127))(then     
			(call $proc_exit (i32.const 170))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.fffffd0000001p+127)) (f32.const 0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 171))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x1.fffffd0000001p+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 172))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.fffffep+127)) (f32.const 0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 173))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x1.fffffep+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 174))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.fffffefffffffp+127)) (f32.const 0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 175))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x1.fffffefffffffp+127)) (f32.const -0x1.fffffep+127))(then     
			(call $proc_exit (i32.const 176))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.ffffffp+127)) (f32.const inf))(then     
			(call $proc_exit (i32.const 177))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x1.ffffffp+127)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 178))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1p-119)) (f32.const 0x1p-119))(then     
			(call $proc_exit (i32.const 179))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.8f867ep+125)) (f32.const 0x1.8f867ep+125))(then     
			(call $proc_exit (i32.const 180))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const inf)) (f32.const inf))(then     
			(call $proc_exit (i32.const 181))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -inf)) (f32.const -inf))(then     
			(call $proc_exit (i32.const 182))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.0000000000001p+0)) (f32.const 1.0))(then     
			(call $proc_exit (i32.const 183))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.fffffffffffffp-1)) (f32.const 1.0))(then     
			(call $proc_exit (i32.const 184))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.0000010000000p+0)) (f32.const 0x1.000000p+0))(then     
			(call $proc_exit (i32.const 185))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.0000010000001p+0)) (f32.const 0x1.000002p+0))(then     
			(call $proc_exit (i32.const 186))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.000002fffffffp+0)) (f32.const 0x1.000002p+0))(then     
			(call $proc_exit (i32.const 187))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.0000030000000p+0)) (f32.const 0x1.000004p+0))(then     
			(call $proc_exit (i32.const 188))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.0000050000000p+0)) (f32.const 0x1.000004p+0))(then     
			(call $proc_exit (i32.const 189))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.0000010000000p+24)) (f32.const 0x1.0p+24))(then     
			(call $proc_exit (i32.const 190))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.0000010000001p+24)) (f32.const 0x1.000002p+24))(then     
			(call $proc_exit (i32.const 191))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.000002fffffffp+24)) (f32.const 0x1.000002p+24))(then     
			(call $proc_exit (i32.const 192))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.0000030000000p+24)) (f32.const 0x1.000004p+24))(then     
			(call $proc_exit (i32.const 193))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.4eae4f7024c7p+108)) (f32.const 0x1.4eae5p+108))(then     
			(call $proc_exit (i32.const 194))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.a12e71e358685p-113)) (f32.const 0x1.a12e72p-113))(then     
			(call $proc_exit (i32.const 195))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.cb98354d521ffp-127)) (f32.const 0x1.cb9834p-127))(then     
			(call $proc_exit (i32.const 196))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x1.6972b30cfb562p+1)) (f32.const -0x1.6972b4p+1))(then     
			(call $proc_exit (i32.const 197))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x1.bedbe4819d4c4p+112)) (f32.const -0x1.bedbe4p+112))(then     
			(call $proc_exit (i32.const 198))
		))

		;; (if (f32.ne (call $f32.demote_f64 (f64.const nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 199))
		;; ))

		;; (if (f32.ne (call $f32.demote_f64 (f64.const nan:0x4000000000000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 200))
		;; ))

		;; (if (f32.ne (call $f32.demote_f64 (f64.const -nan)) (f32.const nan:canonical))(then     
		;; 	(call $proc_exit (i32.const 201))
		;; ))

		;; (if (f32.ne (call $f32.demote_f64 (f64.const -nan:0x4000000000000)) (f32.const nan:arithmetic))(then     
		;; 	(call $proc_exit (i32.const 202))
		;; ))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1p-1022)) (f32.const 0.0))(then     
			(call $proc_exit (i32.const 203))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x1p-1022)) (f32.const -0.0))(then     
			(call $proc_exit (i32.const 204))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.0p-150)) (f32.const 0.0))(then     
			(call $proc_exit (i32.const 205))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x1.0p-150)) (f32.const -0.0))(then     
			(call $proc_exit (i32.const 206))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const 0x1.0000000000001p-150)) (f32.const 0x1p-149))(then     
			(call $proc_exit (i32.const 207))
		))

		(if (f32.ne (call $f32.demote_f64 (f64.const -0x1.0000000000001p-150)) (f32.const -0x1p-149))(then     
			(call $proc_exit (i32.const 208))
		))

		(if (f32.ne (call $f32.reinterpret_i32 (i32.const 0)) (f32.const 0.0)) (then     
			(call $proc_exit (i32.const 209))
		))

		(if (f32.ne (call $f32.reinterpret_i32 (i32.const 0x80000000)) (f32.const -0.0)) (then     
			(call $proc_exit (i32.const 210))
		))

		(if (f32.ne (call $f32.reinterpret_i32 (i32.const 1)) (f32.const 0x1p-149)) (then     
			(call $proc_exit (i32.const 211))
		))

		(if (f32.ne (call $f32.reinterpret_i32 (i32.const -1)) (f32.const -nan:0x7fffff)) (then     
			(call $proc_exit (i32.const 212))
		))

		(if (f32.ne (call $f32.reinterpret_i32 (i32.const 123456789)) (f32.const 0x1.b79a2ap-113)) (then     
			(call $proc_exit (i32.const 213))
		))

		(if (f32.ne (call $f32.reinterpret_i32 (i32.const -2147483647)) (f32.const -0x1p-149)) (then     
			(call $proc_exit (i32.const 214))
		))

		(if (f32.ne (call $f32.reinterpret_i32 (i32.const 0x7f800000)) (f32.const inf)) (then     
			(call $proc_exit (i32.const 215))
		))

		(if (f32.ne (call $f32.reinterpret_i32 (i32.const 0xff800000)) (f32.const -inf)) (then     
			(call $proc_exit (i32.const 216))
		))

		(if (f32.ne (call $f32.reinterpret_i32 (i32.const 0x7fc00000)) (f32.const nan)) (then     
			(call $proc_exit (i32.const 217))
		))

		(if (f32.ne (call $f32.reinterpret_i32 (i32.const 0xffc00000)) (f32.const -nan)) (then     
			(call $proc_exit (i32.const 218))
		))

		(if (f32.ne (call $f32.reinterpret_i32 (i32.const 0x7fa00000)) (f32.const nan:0x200000)) (then     
			(call $proc_exit (i32.const 219))
		))

		(if (f32.ne (call $f32.reinterpret_i32 (i32.const 0xffa00000)) (f32.const -nan:0x200000)) (then     
			(call $proc_exit (i32.const 220))
		))

		(if (f64.ne (call $f64.reinterpret_i64 (i64.const 0)) (f64.const 0.0)) (then     
			(call $proc_exit (i32.const 221))
		))

		(if (f64.ne (call $f64.reinterpret_i64 (i64.const 1)) (f64.const 0x0.0000000000001p-1022)) (then     
			(call $proc_exit (i32.const 222))
		))

		(if (f64.ne (call $f64.reinterpret_i64 (i64.const -1)) (f64.const -nan:0xfffffffffffff)) (then     
			(call $proc_exit (i32.const 223))
		))

		(if (f64.ne (call $f64.reinterpret_i64 (i64.const 0x8000000000000000)) (f64.const -0.0)) (then     
			(call $proc_exit (i32.const 224))
		))

		(if (f64.ne (call $f64.reinterpret_i64 (i64.const 1234567890)) (f64.const 0x0.00000499602d2p-1022)) (then     
			(call $proc_exit (i32.const 225))
		))

		(if (f64.ne (call $f64.reinterpret_i64 (i64.const -9223372036854775807)) (f64.const -0x0.0000000000001p-1022)) (then     
			(call $proc_exit (i32.const 226))
		))

		(if (f64.ne (call $f64.reinterpret_i64 (i64.const 0x7ff0000000000000)) (f64.const inf)) (then     
			(call $proc_exit (i32.const 227))
		))

		(if (f64.ne (call $f64.reinterpret_i64 (i64.const 0xfff0000000000000)) (f64.const -inf)) (then     
			(call $proc_exit (i32.const 228))
		))

		(if (f64.ne (call $f64.reinterpret_i64 (i64.const 0x7ff8000000000000)) (f64.const nan)) (then     
			(call $proc_exit (i32.const 229))
		))

		(if (f64.ne (call $f64.reinterpret_i64 (i64.const 0xfff8000000000000)) (f64.const -nan)) (then     
			(call $proc_exit (i32.const 230))
		))

		(if (f64.ne (call $f64.reinterpret_i64 (i64.const 0x7ff4000000000000)) (f64.const nan:0x4000000000000)) (then     
			(call $proc_exit (i32.const 231))
		))

		(if (f64.ne (call $f64.reinterpret_i64 (i64.const 0xfff4000000000000)) (f64.const -nan:0x4000000000000)) (then     
			(call $proc_exit (i32.const 232))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const 0.0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 233))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const -0.0)) (i64.const 0x8000000000000000)) (then     
			(call $proc_exit (i32.const 234))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const 0x0.0000000000001p-1022)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 235))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const -nan:0xfffffffffffff)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 236))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const -0x0.0000000000001p-1022)) (i64.const 0x8000000000000001)) (then     
			(call $proc_exit (i32.const 237))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const 1.0)) (i64.const 4607182418800017408)) (then     
			(call $proc_exit (i32.const 238))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const 3.14159265358979)) (i64.const 4614256656552045841)) (then     
			(call $proc_exit (i32.const 239))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const 0x1.fffffffffffffp+1023)) (i64.const 9218868437227405311)) (then     
			(call $proc_exit (i32.const 240))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const -0x1.fffffffffffffp+1023)) (i64.const -4503599627370497)) (then     
			(call $proc_exit (i32.const 241))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const inf)) (i64.const 0x7ff0000000000000)) (then     
			(call $proc_exit (i32.const 242))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const -inf)) (i64.const 0xfff0000000000000)) (then     
			(call $proc_exit (i32.const 243))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const nan)) (i64.const 0x7ff8000000000000)) (then     
			(call $proc_exit (i32.const 244))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const -nan)) (i64.const 0xfff8000000000000)) (then     
			(call $proc_exit (i32.const 245))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const nan:0x4000000000000)) (i64.const 0x7ff4000000000000)) (then     
			(call $proc_exit (i32.const 246))
		))

		(if (i64.ne (call $i64.reinterpret_f64 (f64.const -nan:0x4000000000000)) (i64.const 0xfff4000000000000)) (then     
			(call $proc_exit (i32.const 247))
		))



		(call $proc_exit (i32.const 0))
	)	
)






;; ;; Saturating conversions: test all the same values as the non-saturating conversions.

;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const 0.0)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const -0.0)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const 0x1p-149)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const -0x1p-149)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const 1.0)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const 0x1.19999ap+0)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const 1.5)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const -1.0)) (i32.const -1))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const -0x1.19999ap+0)) (i32.const -1))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const -1.5)) (i32.const -1))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const -1.9)) (i32.const -1))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const -2.0)) (i32.const -2))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const 2147483520.0)) (i32.const 2147483520))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const -2147483648.0)) (i32.const -2147483648))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const 2147483648.0)) (i32.const 0x7fffffff))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const -2147483904.0)) (i32.const 0x80000000))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const inf)) (i32.const 0x7fffffff))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const -inf)) (i32.const 0x80000000))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const nan)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const nan:0x200000)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const -nan)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_s" (f32.const -nan:0x200000)) (i32.const 0))

;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const 0.0)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const -0.0)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const 0x1p-149)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const -0x1p-149)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const 1.0)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const 0x1.19999ap+0)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const 1.5)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const 1.9)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const 2.0)) (i32.const 2))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const 2147483648)) (i32.const -2147483648)) ;; 0x1.00000p+31 -> 8000 0000
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const 4294967040.0)) (i32.const -256))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const -0x1.ccccccp-1)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const -0x1.fffffep-1)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const 4294967296.0)) (i32.const 0xffffffff))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const -1.0)) (i32.const 0x00000000))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const inf)) (i32.const 0xffffffff))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const -inf)) (i32.const 0x00000000))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const nan)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const nan:0x200000)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const -nan)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f32_u" (f32.const -nan:0x200000)) (i32.const 0))

;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const 0.0)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const -0.0)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const 0x0.0000000000001p-1022)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const -0x0.0000000000001p-1022)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const 1.0)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const 0x1.199999999999ap+0)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const 1.5)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const -1.0)) (i32.const -1))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const -0x1.199999999999ap+0)) (i32.const -1))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const -1.5)) (i32.const -1))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const -1.9)) (i32.const -1))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const -2.0)) (i32.const -2))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const 2147483647.0)) (i32.const 2147483647))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const -2147483648.0)) (i32.const -2147483648))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const 2147483648.0)) (i32.const 0x7fffffff))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const -2147483649.0)) (i32.const 0x80000000))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const inf)) (i32.const 0x7fffffff))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const -inf)) (i32.const 0x80000000))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const nan)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const nan:0x4000000000000)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const -nan)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_s" (f64.const -nan:0x4000000000000)) (i32.const 0))

;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 0.0)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const -0.0)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 0x0.0000000000001p-1022)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const -0x0.0000000000001p-1022)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 1.0)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 0x1.199999999999ap+0)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 1.5)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 1.9)) (i32.const 1))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 2.0)) (i32.const 2))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 2147483648)) (i32.const -2147483648)) ;; 0x1.00000p+31 -> 8000 0000
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 4294967295.0)) (i32.const -1))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const -0x1.ccccccccccccdp-1)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const -0x1.fffffffffffffp-1)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 1e8)) (i32.const 100000000))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 4294967296.0)) (i32.const 0xffffffff))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const -1.0)) (i32.const 0x00000000))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 1e16)) (i32.const 0xffffffff))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 1e30)) (i32.const 0xffffffff))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const 9223372036854775808)) (i32.const 0xffffffff))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const inf)) (i32.const 0xffffffff))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const -inf)) (i32.const 0x00000000))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const nan)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const nan:0x4000000000000)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const -nan)) (i32.const 0))
;; (assert_return (invoke "i32.trunc_sat_f64_u" (f64.const -nan:0x4000000000000)) (i32.const 0))

;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const 0.0)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const -0.0)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const 0x1p-149)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const -0x1p-149)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const 1.0)) (i64.const 1))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const 0x1.19999ap+0)) (i64.const 1))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const 1.5)) (i64.const 1))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const -1.0)) (i64.const -1))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const -0x1.19999ap+0)) (i64.const -1))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const -1.5)) (i64.const -1))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const -1.9)) (i64.const -1))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const -2.0)) (i64.const -2))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const 4294967296)) (i64.const 4294967296)) ;; 0x1.00000p+32 -> 1 0000 0000
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const -4294967296)) (i64.const -4294967296)) ;; -0x1.00000p+32 -> ffff ffff 0000 0000
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const 9223371487098961920.0)) (i64.const 9223371487098961920))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const -9223372036854775808.0)) (i64.const -9223372036854775808))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const 9223372036854775808.0)) (i64.const 0x7fffffffffffffff))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const -9223373136366403584.0)) (i64.const 0x8000000000000000))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const inf)) (i64.const 0x7fffffffffffffff))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const -inf)) (i64.const 0x8000000000000000))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const nan)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const nan:0x200000)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const -nan)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_s" (f32.const -nan:0x200000)) (i64.const 0))

;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const 0.0)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const -0.0)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const 0x1p-149)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const -0x1p-149)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const 1.0)) (i64.const 1))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const 0x1.19999ap+0)) (i64.const 1))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const 1.5)) (i64.const 1))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const 4294967296)) (i64.const 4294967296))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const 18446742974197923840.0)) (i64.const -1099511627776))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const -0x1.ccccccp-1)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const -0x1.fffffep-1)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const 18446744073709551616.0)) (i64.const 0xffffffffffffffff))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const -1.0)) (i64.const 0x0000000000000000))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const inf)) (i64.const 0xffffffffffffffff))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const -inf)) (i64.const 0x0000000000000000))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const nan)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const nan:0x200000)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const -nan)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f32_u" (f32.const -nan:0x200000)) (i64.const 0))

;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const 0.0)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const -0.0)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const 0x0.0000000000001p-1022)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const -0x0.0000000000001p-1022)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const 1.0)) (i64.const 1))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const 0x1.199999999999ap+0)) (i64.const 1))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const 1.5)) (i64.const 1))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const -1.0)) (i64.const -1))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const -0x1.199999999999ap+0)) (i64.const -1))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const -1.5)) (i64.const -1))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const -1.9)) (i64.const -1))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const -2.0)) (i64.const -2))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const 4294967296)) (i64.const 4294967296)) ;; 0x1.00000p+32 -> 1 0000 0000
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const -4294967296)) (i64.const -4294967296)) ;; -0x1.00000p+32 -> ffff ffff 0000 0000
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const 9223372036854774784.0)) (i64.const 9223372036854774784))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const -9223372036854775808.0)) (i64.const -9223372036854775808))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const 9223372036854775808.0)) (i64.const 0x7fffffffffffffff))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const -9223372036854777856.0)) (i64.const 0x8000000000000000))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const inf)) (i64.const 0x7fffffffffffffff))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const -inf)) (i64.const 0x8000000000000000))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const nan)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const nan:0x4000000000000)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const -nan)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_s" (f64.const -nan:0x4000000000000)) (i64.const 0))

;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const 0.0)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const -0.0)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const 0x0.0000000000001p-1022)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const -0x0.0000000000001p-1022)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const 1.0)) (i64.const 1))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const 0x1.199999999999ap+0)) (i64.const 1))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const 1.5)) (i64.const 1))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const 4294967295)) (i64.const 0xffffffff))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const 4294967296)) (i64.const 0x100000000))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const 18446744073709549568.0)) (i64.const -2048))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const -0x1.ccccccccccccdp-1)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const -0x1.fffffffffffffp-1)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const 1e8)) (i64.const 100000000))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const 1e16)) (i64.const 10000000000000000))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const 9223372036854775808)) (i64.const -9223372036854775808))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const 18446744073709551616.0)) (i64.const 0xffffffffffffffff))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const -1.0)) (i64.const 0x0000000000000000))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const inf)) (i64.const 0xffffffffffffffff))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const -inf)) (i64.const 0x0000000000000000))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const nan)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const nan:0x4000000000000)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const -nan)) (i64.const 0))
;; (assert_return (invoke "i64.trunc_sat_f64_u" (f64.const -nan:0x4000000000000)) (i64.const 0))

