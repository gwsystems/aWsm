;; This test is original, not a port of the wasm test suite
;; It was created as a simplification of br_table__as_loop_first

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory (export "memory") 1)

	(func $dummy)

	(func $as-block-value-deep (export "as-block-value-deep") (result i32)
		(block (result i32) 
			(i32.const 2)
			(br 0)
			;; Unreachable
			(i32.const 42)
		)
	)

    (func (export "_start")
		(if (i32.ne (call $as-block-value-deep) (i32.const 2))(then     
			(call $proc_exit (i32.const 1))
		))

        (call $proc_exit (i32.const 0))
    )
)
