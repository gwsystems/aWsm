;; data.drop
(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)

	;; error: passive data segments are not allowed
	(data $p "x")
	(data $a (memory 0) (i32.const 0) "x")

	(func $drop_passive (export "drop_passive") (data.drop $p))
	(func $init_passive (export "init_passive") (param $len i32)
		(memory.init $p (i32.const 0) (i32.const 0) (local.get $len))
	)

	(func $drop_active (export "drop_active") (data.drop $a))
	(func $init_active (export "init_active") (param $len i32)
		(memory.init $a (i32.const 0) (i32.const 0) (local.get $len))
	)

	(func (export "_start")
		(call $init_passive (i32.const 1))
		(call $drop_passive)
		(call $drop_passive)
		(call $init_passive (i32.const 0))
		;; (assert_trap (invoke "init_passive" (i32.const 1)) "out of bounds memory access")
		(call $init_passive (i32.const 0))
		(call $drop_active)
		(call $init_active (i32.const 0))
		;; (assert_trap (invoke "init_active" (i32.const 1)) "out of bounds memory access")
		(call $init_active (i32.const 0))
	)
)

