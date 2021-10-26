;; memory.fill
(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)

	(func $fill (export "fill") (param i32 i32 i32)
		(memory.fill
		(local.get 0)
		(local.get 1)
		(local.get 2)))

	(func $load8_u (export "load8_u") (param i32) (result i32)
		(i32.load8_u (local.get 0))
	)
	
	(func (export "_start") (param i32)
		(call $invoke (i32.const 1) (i32.const 0xff) (i32.const 3))

		;; Basic fill test.
		(if (i32.ne (call $load8_u (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $load8_u (i32.const 1)) (i32.const 0xff)) (then     
			(call $proc_exit (i32.const 2))
		))

		(if (i32.ne (call $load8_u (i32.const 2)) (i32.const 0xff)) (then     
			(call $proc_exit (i32.const 3))
		))

		(if (i32.ne (call $load8_u (i32.const 3)) (i32.const 0xff)) (then     
			(call $proc_exit (i32.const 4))
		))

		(if (i32.ne (call $load8_u (i32.const 4)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 5))
		))

		;; Fill value is stored as a byte.
		(call $fill (i32.const 0) (i32.const 0xbbaa) (i32.const 2))

		(if (i32.ne (call $load8_u (i32.const 0)) (i32.const 0xaa)) (then     
			(call $proc_exit (i32.const 6))
		))

		(if (i32.ne (call $load8_u (i32.const 1)) (i32.const 0xaa)) (then     
			(call $proc_exit (i32.const 7))
		))

		;; Fill all of memory
		(call $fill (i32.const 0) (i32.const 0) (i32.const 0x10000))

		;; Out-of-bounds writes trap, and nothing is written
		;; (assert_trap (invoke "fill" (i32.const 0xff00) (i32.const 1) (i32.const 0x101))
		;;     "out of bounds memory access")

		(if (i32.ne (call $load8_u (i32.const 0xff00)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 8))
		))

		(if (i32.ne (call $load8_u (i32.const 0xffff)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 9))
		))

		;; Succeed when writing 0 bytes at the end of the region.
		(call $fill (i32.const 0x10000) (i32.const 0) (i32.const 0))

		;; Writing 0 bytes outside the memory traps.
		;; (assert_trap (invoke "fill" (i32.const 0x10001) (i32.const 0) (i32.const 0))
		;;     "out of bounds memory access")

		(call $proc_exit (i32.const 0))
	)
)


