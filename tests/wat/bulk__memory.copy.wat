;; memory.copy
(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory (data "\aa\bb\cc\dd"))

	(func (export "copy") (param i32 i32 i32)
		(memory.copy
		(local.get 0)
		(local.get 1)
		(local.get 2))
	)

	(func (export "load8_u") (param i32) (result i32)
		(i32.load8_u (local.get 0))
	)

	(func (export "_start") (param i32)
		;; Non-overlapping copy
		(call $copy (i32.const 10) (i32.const 0) (i32.const 4))

		(if (i32.ne (call $load8_u (i32.const 9)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $load8_u (i32.const 10)) (i32.const 0xaa)) (then     
			(call $proc_exit (i32.const 2))
		))

		(if (i32.ne (call $load8_u (i32.const 11)) (i32.const 0xbb)) (then     
			(call $proc_exit (i32.const 3))
		))

		(if (i32.ne (call $load8_u (i32.const 12)) (i32.const 0xcc)) (then     
			(call $proc_exit (i32.const 4))
		))

		(if (i32.ne (call $load8_u (i32.const 13)) (i32.const 0xdd)) (then     
			(call $proc_exit (i32.const 5))
		))

		(if (i32.ne (call $load8_u (i32.const 14)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 6))
		))

		;; Overlap, source > dest
		(call $copy (i32.const 8) (i32.const 10) (i32.const 4))

		(if (i32.ne (call $load8_u (i32.const 8)) (i32.const 0xaa))(then     
			(call $proc_exit (i32.const 5))
		))

		(if (i32.ne (call $load8_u (i32.const 9)) (i32.const 0xbb))(then     
			(call $proc_exit (i32.const 5))
		))

		(if (i32.ne (call $load8_u (i32.const 10)) (i32.const 0xcc))(then     
			(call $proc_exit (i32.const 5))
		))

		(if (i32.ne (call $load8_u (i32.const 11)) (i32.const 0xdd))(then     
			(call $proc_exit (i32.const 5))
		))

		(if (i32.ne (call $load8_u (i32.const 12)) (i32.const 0xcc))(then     
			(call $proc_exit (i32.const 5))
		))

		(if (i32.ne (call $load8_u (i32.const 13)) (i32.const 0xdd))(then     
			(call $proc_exit (i32.const 5))
		))

		;; Overlap, source < dest
		(call $copy (i32.const 10) (i32.const 7) (i32.const 6))

		(if (i32.ne (call $load8_u (i32.const 10)) (i32.const 0))(then     
			(call $proc_exit (i32.const 5))
		))
		
		(if (i32.ne (call $load8_u (i32.const 11)) (i32.const 0xaa))(then     
			(call $proc_exit (i32.const 5))
		))

		(if (i32.ne (call $load8_u (i32.const 12)) (i32.const 0xbb))(then     
			(call $proc_exit (i32.const 5))
		))
		
		(if (i32.ne (call $load8_u (i32.const 13)) (i32.const 0xcc))(then     
			(call $proc_exit (i32.const 5))
		))
		
		(if (i32.ne (call $load8_u (i32.const 14)) (i32.const 0xdd))(then     
			(call $proc_exit (i32.const 5))
		))
		
		(if (i32.ne (call $load8_u (i32.const 15)) (i32.const 0xcc))(then     
			(call $proc_exit (i32.const 5))
		))
		
		(if (i32.ne (call $load8_u (i32.const 16)) (i32.const 0))(then     
			(call $proc_exit (i32.const 5))
		))

		;; Copy ending at memory limit is ok.
		(call $copy (i32.const 0xff00) (i32.const 0) (i32.const 0x100))
		(call $copy (i32.const 0xfe00) (i32.const 0xff00) (i32.const 0x100))

		;; Succeed when copying 0 bytes at the end of the region.
		(call $copy (i32.const 0x10000) (i32.const 0) (i32.const 0))
		(call $copy (i32.const 0) (i32.const 0x10000) (i32.const 0))

		;; Copying 0 bytes outside the memory traps.
		;; (assert_trap (invoke "copy" (i32.const 0x10001) (i32.const 0) (i32.const 0))
		;;     "out of bounds memory access")
		;; (assert_trap (invoke "copy" (i32.const 0) (i32.const 0x10001) (i32.const 0))
		;;     "out of bounds memory access")

		(call $proc_exit (i32.const 0))
	)
)
