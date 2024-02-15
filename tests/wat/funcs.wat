(module $coolModule
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 2)
	(export "memory" (memory 0))
	(table $tbl 0 funcref)

	(func $secretFunc (result i32)
		i32.const 5
	)

	(func $magicNumber (result i32)
		i32.const 42
	)

	(func $mult (param $first i32) (param $second i32) (result i32)
		local.get 0
		local.get 1
		i32.mul
	)

	(func $partial (param $first i32) (param i32) (result i32)
		local.get 0
		local.get 1
		i32.mul
	)

	(func $willGetOverwritten (export "_start")
		(if (i32.ne (call $mult (call $secretFunc) (call $magicNumber)) (i32.const 210)) (then
			(call $proc_exit (i32.const 1))
		) (else 
			(call $proc_exit (i32.const 0))
		))
	)
)
