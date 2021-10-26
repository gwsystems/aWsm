;; table.init
(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(table 3 funcref)
	(elem funcref
		(ref.func $zero) (ref.func $one) (ref.func $zero) (ref.func $one))

	(func $zero (result i32) (i32.const 0))
	(func $one (result i32) (i32.const 1))

	(func (export "init") (param i32 i32 i32)
		(table.init 0
		(local.get 0)
		(local.get 1)
		(local.get 2))
	)

	(func (export "call") (param i32) (result i32)
		(call_indirect (result i32)
		(local.get 0))
	)

	(func (export "_start")
		;; Out-of-bounds stores trap, and nothing is written.
		;; (assert_trap (invoke "init" (i32.const 2) (i32.const 0) (i32.const 2))
		;; 	"out of bounds table access")
		;; (assert_trap (invoke "call" (i32.const 2))
		;; 	"uninitialized element 2")

		(call $init (i32.const 0) (i32.const 1) (i32.const 2))
		(if (i32.ne (call $call (i32.const 0)) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $call (i32.const 1)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 2))
		))
		;; (assert_trap (invoke "call" (i32.const 2)) "uninitialized element")

		;; Init ending at table limit and segment limit is ok.
		(call $init (i32.const 1) (i32.const 2) (i32.const 2))

		;; Succeed when storing 0 elements at the end of either region.
		(call $init (i32.const 3) (i32.const 0) (i32.const 0))
		(call $init (i32.const 0) (i32.const 4) (i32.const 0))

		;; Writing 0 elements outside the table traps.
		;; (assert_trap (invoke "init" (i32.const 4) (i32.const 0) (i32.const 0))
		;; 	"out of bounds table access")
		;; (assert_trap (invoke "init" (i32.const 0) (i32.const 5) (i32.const 0))
		;; 	"out of bounds table access")

		(call $proc_exit (i32.const 0))
	)
)

