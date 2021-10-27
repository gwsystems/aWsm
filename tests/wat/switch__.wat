(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
	;; Statement switch
	(func $stmt (export "stmt") (param $i i32) (result i32)
		(local $j i32)
		(local.set $j (i32.const 100))
		(block $switch
		(block $7
			(block $default
			(block $6
				(block $5
				(block $4
					(block $3
					(block $2
						(block $1
						(block $0
							(br_table $0 $1 $2 $3 $4 $5 $6 $7 $default
							(local.get $i)
							)
						) ;; 0
						(return (local.get $i))
						) ;; 1
						(nop)
						;; fallthrough
					) ;; 2
					;; fallthrough
					) ;; 3
					(local.set $j (i32.sub (i32.const 0) (local.get $i)))
					(br $switch)
				) ;; 4
				(br $switch)
				) ;; 5
				(local.set $j (i32.const 101))
				(br $switch)
			) ;; 6
			(local.set $j (i32.const 101))
			;; fallthrough
			) ;; default
			(local.set $j (i32.const 102))
		) ;; 7
		;; fallthrough
		)
		(return (local.get $j))
	)

	;; Expression switch
	(func $expr (export "expr") (param $i i64) (result i64)
		(local $j i64)
		(local.set $j (i64.const 100))
		(return
		(block $switch (result i64)
			(block $7
			(block $default
				(block $4
				(block $5
					(block $6
					(block $3
						(block $2
						(block $1
							(block $0
							(br_table $0 $1 $2 $3 $4 $5 $6 $7 $default
								(i32.wrap_i64 (local.get $i))
							)
							) ;; 0
							(return (local.get $i))
						) ;; 1
						(nop)
						;; fallthrough
						) ;; 2
						;; fallthrough
					) ;; 3
					(br $switch (i64.sub (i64.const 0) (local.get $i)))
					) ;; 6
					(local.set $j (i64.const 101))
					;; fallthrough
				) ;; 4
				;; fallthrough
				) ;; 5
				;; fallthrough
			) ;; default
			(br $switch (local.get $j))
			) ;; 7
			(i64.const -5)
		)
		)
	)

	;; Argument switch
	(func $arg (export "arg") (param $i i32) (result i32)
		(return
		(block $2 (result i32)
			(i32.add (i32.const 10)
			(block $1 (result i32)
				(i32.add (i32.const 100)
				(block $0 (result i32)
					(i32.add (i32.const 1000)
					(block $default (result i32)
						(br_table $0 $1 $2 $default
						(i32.mul (i32.const 2) (local.get $i))
						(i32.and (i32.const 3) (local.get $i))
						)
					)
					)
				)
				)
			)
			)
		)
		)
	)

	;; Corner cases
	(func $corner (export "corner") (result i32)
		(block
		(br_table 0 (i32.const 0))
		)
		(i32.const 1)
	)

	(func $_start (export "_start")

		(if (i32.ne (call $stmt (i32.const 0)) (i32.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $stmt (i32.const 1)) (i32.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $stmt (i32.const 2)) (i32.const -2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $stmt (i32.const 3)) (i32.const -3)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $stmt (i32.const 4)) (i32.const 100)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $stmt (i32.const 5)) (i32.const 101)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $stmt (i32.const 6)) (i32.const 102)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $stmt (i32.const 7)) (i32.const 100)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $stmt (i32.const -10)) (i32.const 102)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $expr (i64.const 0)) (i64.const 0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $expr (i64.const 1)) (i64.const -1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $expr (i64.const 2)) (i64.const -2)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $expr (i64.const 3)) (i64.const -3)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $expr (i64.const 6)) (i64.const 101)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $expr (i64.const 7)) (i64.const -5)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i64.ne (call $expr (i64.const -10)) (i64.const 100)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i32.ne (call $arg (i32.const 0)) (i32.const 110)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i32.ne (call $arg (i32.const 1)) (i32.const 12)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i32.ne (call $arg (i32.const 2)) (i32.const 4)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (i32.ne (call $arg (i32.const 3)) (i32.const 1116)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i32.ne (call $arg (i32.const 4)) (i32.const 118)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i32.ne (call $arg (i32.const 5)) (i32.const 20)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i32.ne (call $arg (i32.const 6)) (i32.const 12)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i32.ne (call $arg (i32.const 7)) (i32.const 1124)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i32.ne (call $arg (i32.const 8)) (i32.const 126)) (then     
			(call $proc_exit (i32.const 1))
		))
		
		(if (i32.ne (call $corner) (i32.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

		(call $proc_exit (i32.const 0))
	)
)
