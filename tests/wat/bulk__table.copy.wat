;; table.copy
(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(table 10 funcref)
	(elem (i32.const 0) $zero $one $two)
	(func $zero (result i32) (i32.const 0))
	(func $one (result i32) (i32.const 1))
	(func $two (result i32) (i32.const 2))

	(func (export "copy") (param i32 i32 i32)
		(table.copy
		(local.get 0)
		(local.get 1)
		(local.get 2))
	)

	(func (export "call") (param i32) (result i32)
		(call_indirect (result i32)
		(local.get 0))
	)

	(func (export "_start")
		;; Non-overlapping copy.
		(call $copy (i32.const 3) (i32.const 0) (i32.const 3))

		;; Now [$zero, $one, $two, $zero, $one, $two, ...]
		(if (i32.ne (call $call (i32.const 3)) (i32.const 0) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $call (i32.const 4)) (i32.const 1) (then     
			(call $proc_exit (i32.const 2))
		))

		(if (i32.ne (call $call (i32.const 5)) (i32.const 2) (then     
			(call $proc_exit (i32.const 3))
		))

		;; Overlap, source > dest
		(call $copy (i32.const 0) (i32.const 1) (i32.const 3))

		;; Now [$one, $two, $zero, $zero, $one, $two, ...]
		(if (i32.ne (call $call (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 4))
		))
		
		(if (i32.ne (call $call (i32.const 1)) (i32.const 2)) (then     
			(call $proc_exit (i32.const 5))
		))

		(if (i32.ne (call $call (i32.const 2)) (i32.const 0))  (then     
			(call $proc_exit (i32.const 6))
		))

		;; Overlap, source < dest
		(call $copy (i32.const 2) (i32.const 0) (i32.const 3))
		;; Now [$one, $two, $one, $two, $zero, $two, ...]

		(if (i32.ne (call $call (i32.const 2)) (i32.const 1))  (then     
			(call $proc_exit (i32.const 7))
		))

		(if (i32.ne (call $call (i32.const 3)) (i32.const 2))  (then     
			(call $proc_exit (i32.const 8))
		))

		(if (i32.ne (call $call (i32.const 4)) (i32.const 0))  (then     
			(call $proc_exit (i32.const 9))
		))

		;; Copy ending at table limit is ok.
		(call $copy (i32.const 6) (i32.const 8) (i32.const 2))
		(call $copy (i32.const 8) (i32.const 6) (i32.const 2))

		;; Succeed when copying 0 elements at the end of the region.
		(call $copy (i32.const 10) (i32.const 0) (i32.const 0))
		(call $copy (i32.const 0) (i32.const 10) (i32.const 0))

		;; Fail on out-of-bounds when copying 0 elements outside of table.
		;; (assert_trap (invoke "copy" (i32.const 11) (i32.const 0) (i32.const 0))
		;;   "out of bounds table access")
		;; (assert_trap (invoke "copy" (i32.const 0) (i32.const 11) (i32.const 0))
		;;   "out of bounds table access")

		(call $proc_exit (i32.const 0))
	)
)

