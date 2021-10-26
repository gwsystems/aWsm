;; memory.init
(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
	(data "\aa\bb\cc\dd")

	(func $init (export "init") (param i32 i32 i32)
		(memory.init 0
		(local.get 0)
		(local.get 1)
		(local.get 2))
	)

	(func $load8_u (export "load8_u") (param i32) (result i32)
		(i32.load8_u (local.get 0)))

	(func (export "_start")
		(call $init (i32.const 0) (i32.const 1) (i32.const 2))

		(if (i32.ne (call $load8_u (i32.const 0)) (i32.const 0xbb)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $load8_u (i32.const 1)) (i32.const 0xcc)) (then     
			(call $proc_exit (i32.const 2))
		))

		(if (i32.ne (call $load8_u (i32.const 2)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 3))
		))

		;; Init ending at memory limit and segment limit is ok.
		(call $init (i32.const 0xfffc) (i32.const 0) (i32.const 4))

		;; Out-of-bounds writes trap, and nothing is written.
		;; (assert_trap (invoke "init" (i32.const 0xfffe) (i32.const 0) (i32.const 3))
		;;     "out of bounds memory access")


		(if (i32.ne (call $load8_u (i32.const 0xfffe)) (i32.const 0xcc)) (then     
			(call $proc_exit (i32.const 4))
		))

		(if (i32.ne (call $load8_u (i32.const 0xffff)) (i32.const 0xdd)) (then     
			(call $proc_exit (i32.const 5))
		))

		;; Succeed when writing 0 bytes at the end of either region.
		(call $init (i32.const 0x10000) (i32.const 0) (i32.const 0))
		(call $init (i32.const 0) (i32.const 4) (i32.const 0))

		;; Writing 0 bytes outside the memory traps.
		;; (assert_trap (invoke "init" (i32.const 0x10001) (i32.const 0) (i32.const 0))
		;;     "out of bounds memory access")
		;; (assert_trap (invoke "init" (i32.const 0) (i32.const 5) (i32.const 0))
		;;     "out of bounds memory access")

		(call $proc_exit (i32.const 0))
	)
)
