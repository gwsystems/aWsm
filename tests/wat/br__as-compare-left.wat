(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    (memory 1)
	
	(func $as-compare-left (export "as-compare-left") (result i32)
		(block (result i32) (f64.le (br 0 (i32.const 43)) (f64.const 10)))
	)

    (func (export "_start") (param i32)
		(call $as-compare-left (i32.const 43))
		drop
		drop

        (call $proc_exit (i32.const 0))
    )
)
