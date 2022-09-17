;; Test aligned and unaligned read/write

(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(memory 1)
	(export "memory" (memory 0))

  ;; $default: natural alignment, $1: align=1, $2: align=2, $4: align=4, $8: align=8

	(func $f32_align_switch (export "f32_align_switch") (param i32) (result f32)
		(local f32 f32)
		(local.set 1 (f32.const 10.0))
		(block $4
			(block $2
				(block $1
					(block $default
						(block $0
						(br_table $0 $default $1 $2 $4 (local.get 0))
						) ;; 0
						(f32.store (i32.const 0) (local.get 1))
						(local.set 2 (f32.load (i32.const 0)))
						(br $4)
					) ;; default
				(f32.store align=1 (i32.const 0) (local.get 1))
				(local.set 2 (f32.load align=1 (i32.const 0)))
				(br $4)
				) ;; 1
				(f32.store align=2 (i32.const 0) (local.get 1))
				(local.set 2 (f32.load align=2 (i32.const 0)))
				(br $4)
			) ;; 2
			(f32.store align=4 (i32.const 0) (local.get 1))
			(local.set 2 (f32.load align=4 (i32.const 0)))
		) ;; 4
		(local.get 2)
	)

	(func $f64_align_switch (export "f64_align_switch") (param i32) (result f64)
		(local f64 f64)
		(local.set 1 (f64.const 10.0))
		(block $8
			(block $4
				(block $2
					(block $1
						(block $default
							(block $0
								(br_table $0 $default $1 $2 $4 $8 (local.get 0))
							) ;; 0
							(f64.store (i32.const 0) (local.get 1))
							(local.set 2 (f64.load (i32.const 0)))
							(br $8)
						) ;; default
						(f64.store align=1 (i32.const 0) (local.get 1))
						(local.set 2 (f64.load align=1 (i32.const 0)))
						(br $8)
					) ;; 1
					(f64.store align=2 (i32.const 0) (local.get 1))
					(local.set 2 (f64.load align=2 (i32.const 0)))
					(br $8)
				) ;; 2
				(f64.store align=4 (i32.const 0) (local.get 1))
				(local.set 2 (f64.load align=4 (i32.const 0)))
				(br $8)
			) ;; 4
			(f64.store align=8 (i32.const 0) (local.get 1))
			(local.set 2 (f64.load align=8 (i32.const 0)))
		) ;; 8
		(local.get 2)
	)

  ;; $8s: i32/i64.load8_s, $8u: i32/i64.load8_u, $16s: i32/i64.load16_s, $16u: i32/i64.load16_u, $32: i32.load
  ;; $32s: i64.load32_s, $32u: i64.load32_u, $64: i64.load

	(func $i32_align_switch (export "i32_align_switch") (param i32 i32) (result i32)
		(local i32 i32)
		(local.set 2 (i32.const 10))
		(block $32
			(block $16u
				(block $16s
				(block $8u
					(block $8s
					(block $0
						(br_table $0 $8s $8u $16s $16u $32 (local.get 0))
					) ;; 0
					(if (i32.eq (local.get 1) (i32.const 0))
						(then
						(i32.store8 (i32.const 0) (local.get 2))
						(local.set 3 (i32.load8_s (i32.const 0)))
						)
					)
					(if (i32.eq (local.get 1) (i32.const 1))
						(then
						(i32.store8 align=1 (i32.const 0) (local.get 2))
						(local.set 3 (i32.load8_s align=1 (i32.const 0)))
						)
					)
					(br $32)
					) ;; 8s
					(if (i32.eq (local.get 1) (i32.const 0))
					(then
						(i32.store8 (i32.const 0) (local.get 2))
						(local.set 3 (i32.load8_u (i32.const 0)))
					)
					)
					(if (i32.eq (local.get 1) (i32.const 1))
					(then
						(i32.store8 align=1 (i32.const 0) (local.get 2))
						(local.set 3 (i32.load8_u align=1 (i32.const 0)))
					)
					)
					(br $32)
				) ;; 8u
				(if (i32.eq (local.get 1) (i32.const 0))
					(then
					(i32.store16 (i32.const 0) (local.get 2))
					(local.set 3 (i32.load16_s (i32.const 0)))
					)
				)
				(if (i32.eq (local.get 1) (i32.const 1))
					(then
					(i32.store16 align=1 (i32.const 0) (local.get 2))
					(local.set 3 (i32.load16_s align=1 (i32.const 0)))
					)
				)
				(if (i32.eq (local.get 1) (i32.const 2))
					(then
					(i32.store16 align=2 (i32.const 0) (local.get 2))
					(local.set 3 (i32.load16_s align=2 (i32.const 0)))
					)
				)
				(br $32)
				) ;; 16s
				(if (i32.eq (local.get 1) (i32.const 0))
				(then
					(i32.store16 (i32.const 0) (local.get 2))
					(local.set 3 (i32.load16_u (i32.const 0)))
				)
				)
				(if (i32.eq (local.get 1) (i32.const 1))
				(then
					(i32.store16 align=1 (i32.const 0) (local.get 2))
					(local.set 3 (i32.load16_u align=1 (i32.const 0)))
				)
				)
				(if (i32.eq (local.get 1) (i32.const 2))
				(then
					(i32.store16 align=2 (i32.const 0) (local.get 2))
					(local.set 3 (i32.load16_u align=2 (i32.const 0)))
				)
				)
				(br $32)
			) ;; 16u
			(if (i32.eq (local.get 1) (i32.const 0))
				(then
				(i32.store (i32.const 0) (local.get 2))
				(local.set 3 (i32.load (i32.const 0)))
				)
			)
			(if (i32.eq (local.get 1) (i32.const 1))
				(then
				(i32.store align=1 (i32.const 0) (local.get 2))
				(local.set 3 (i32.load align=1 (i32.const 0)))
				)
			)
			(if (i32.eq (local.get 1) (i32.const 2))
				(then
				(i32.store align=2 (i32.const 0) (local.get 2))
				(local.set 3 (i32.load align=2 (i32.const 0)))
				)
			)
			(if (i32.eq (local.get 1) (i32.const 4))
				(then
				(i32.store align=4 (i32.const 0) (local.get 2))
				(local.set 3 (i32.load align=4 (i32.const 0)))
				)
			)
		) ;; 32
		(local.get 3)
	)

	(func $i64_align_switch (export "i64_align_switch") (param i32 i32) (result i64)
		(local i64 i64)
		(local.set 2 (i64.const 10))
		(block $64
		(block $32u
			(block $32s
			(block $16u
				(block $16s
				(block $8u
					(block $8s
					(block $0
						(br_table $0 $8s $8u $16s $16u $32s $32u $64 (local.get 0))
					) ;; 0
					(if (i32.eq (local.get 1) (i32.const 0))
						(then
						(i64.store8 (i32.const 0) (local.get 2))
						(local.set 3 (i64.load8_s (i32.const 0)))
						)
					)
					(if (i32.eq (local.get 1) (i32.const 1))
						(then
						(i64.store8 align=1 (i32.const 0) (local.get 2))
						(local.set 3 (i64.load8_s align=1 (i32.const 0)))
						)
					)
					(br $64)
					) ;; 8s
					(if (i32.eq (local.get 1) (i32.const 0))
					(then
						(i64.store8 (i32.const 0) (local.get 2))
						(local.set 3 (i64.load8_u (i32.const 0)))
					)
					)
					(if (i32.eq (local.get 1) (i32.const 1))
					(then
						(i64.store8 align=1 (i32.const 0) (local.get 2))
						(local.set 3 (i64.load8_u align=1 (i32.const 0)))
					)
					)
					(br $64)
				) ;; 8u
				(if (i32.eq (local.get 1) (i32.const 0))
					(then
					(i64.store16 (i32.const 0) (local.get 2))
					(local.set 3 (i64.load16_s (i32.const 0)))
					)
				)
				(if (i32.eq (local.get 1) (i32.const 1))
					(then
					(i64.store16 align=1 (i32.const 0) (local.get 2))
					(local.set 3 (i64.load16_s align=1 (i32.const 0)))
					)
				)
				(if (i32.eq (local.get 1) (i32.const 2))
					(then
					(i64.store16 align=2 (i32.const 0) (local.get 2))
					(local.set 3 (i64.load16_s align=2 (i32.const 0)))
					)
				)
				(br $64)
				) ;; 16s
				(if (i32.eq (local.get 1) (i32.const 0))
				(then
					(i64.store16 (i32.const 0) (local.get 2))
					(local.set 3 (i64.load16_u (i32.const 0)))
				)
				)
				(if (i32.eq (local.get 1) (i32.const 1))
				(then
					(i64.store16 align=1 (i32.const 0) (local.get 2))
					(local.set 3 (i64.load16_u align=1 (i32.const 0)))
				)
				)
				(if (i32.eq (local.get 1) (i32.const 2))
				(then
					(i64.store16 align=2 (i32.const 0) (local.get 2))
					(local.set 3 (i64.load16_u align=2 (i32.const 0)))
				)
				)
				(br $64)
			) ;; 16u
			(if (i32.eq (local.get 1) (i32.const 0))
				(then
				(i64.store32 (i32.const 0) (local.get 2))
				(local.set 3 (i64.load32_s (i32.const 0)))
				)
			)
			(if (i32.eq (local.get 1) (i32.const 1))
				(then
				(i64.store32 align=1 (i32.const 0) (local.get 2))
				(local.set 3 (i64.load32_s align=1 (i32.const 0)))
				)
			)
			(if (i32.eq (local.get 1) (i32.const 2))
				(then
				(i64.store32 align=2 (i32.const 0) (local.get 2))
				(local.set 3 (i64.load32_s align=2 (i32.const 0)))
				)
			)
			(if (i32.eq (local.get 1) (i32.const 4))
				(then
				(i64.store32 align=4 (i32.const 0) (local.get 2))
				(local.set 3 (i64.load32_s align=4 (i32.const 0)))
				)
			)
			(br $64)
			) ;; 32s
			(if (i32.eq (local.get 1) (i32.const 0))
			(then
				(i64.store32 (i32.const 0) (local.get 2))
				(local.set 3 (i64.load32_u (i32.const 0)))
			)
			)
			(if (i32.eq (local.get 1) (i32.const 1))
			(then
				(i64.store32 align=1 (i32.const 0) (local.get 2))
				(local.set 3 (i64.load32_u align=1 (i32.const 0)))
			)
			)
			(if (i32.eq (local.get 1) (i32.const 2))
			(then
				(i64.store32 align=2 (i32.const 0) (local.get 2))
				(local.set 3 (i64.load32_u align=2 (i32.const 0)))
			)
			)
			(if (i32.eq (local.get 1) (i32.const 4))
			(then
				(i64.store32 align=4 (i32.const 0) (local.get 2))
				(local.set 3 (i64.load32_u align=4 (i32.const 0)))
			)
			)
			(br $64)
		) ;; 32u
		(if (i32.eq (local.get 1) (i32.const 0))
			(then
			(i64.store (i32.const 0) (local.get 2))
			(local.set 3 (i64.load (i32.const 0)))
			)
		)
		(if (i32.eq (local.get 1) (i32.const 1))
			(then
			(i64.store align=1 (i32.const 0) (local.get 2))
			(local.set 3 (i64.load align=1 (i32.const 0)))
			)
		)
		(if (i32.eq (local.get 1) (i32.const 2))
			(then
			(i64.store align=2 (i32.const 0) (local.get 2))
			(local.set 3 (i64.load align=2 (i32.const 0)))
			)
		)
		(if (i32.eq (local.get 1) (i32.const 4))
			(then
			(i64.store align=4 (i32.const 0) (local.get 2))
			(local.set 3 (i64.load align=4 (i32.const 0)))
			)
		)
		(if (i32.eq (local.get 1) (i32.const 8))
			(then
			(i64.store align=8 (i32.const 0) (local.get 2))
			(local.set 3 (i64.load align=8 (i32.const 0)))
			)
		)
		) ;; 64
		(local.get 3)
	)

	(func (export "_start") (param i32)
		(if (f32.ne (call $f32_align_switch (i32.const 0)) (f32.const 10.0)) (then     
			(call $proc_exit (i32.const 1))
		))

		(if (f32.ne (call $f32_align_switch (i32.const 1)) (f32.const 10.0)) (then     
			(call $proc_exit (i32.const 2))
		))
		
		(if (f32.ne (call $f32_align_switch (i32.const 3)) (f32.const 10.0)) (then     
			(call $proc_exit (i32.const 3))
		))
		
		(if (f64.ne (call $f64_align_switch (i32.const 0)) (f64.const 10.0)) (then     
			(call $proc_exit (i32.const 5))
		))

		(if (f64.ne (call $f64_align_switch (i32.const 1)) (f64.const 10.0)) (then     
			(call $proc_exit (i32.const 6))
		))
		
		(if (f64.ne (call $f64_align_switch (i32.const 3)) (f64.const 10.0)) (then     
			(call $proc_exit (i32.const 7))
		))
		
		(if (f64.ne (call $f64_align_switch (i32.const 4)) (f64.const 10.0)) (then     
			(call $proc_exit (i32.const 8))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 0) (i32.const 0)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 9))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 0) (i32.const 1)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 10))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 1) (i32.const 0)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 11))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 1) (i32.const 1)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 12))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 2) (i32.const 0)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 13))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 2) (i32.const 1)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 14))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 2) (i32.const 2)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 15))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 3) (i32.const 0)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 16))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 3) (i32.const 1)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 17))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 3) (i32.const 2)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 18))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 4) (i32.const 0)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 19))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 4) (i32.const 1)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 20))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 4) (i32.const 2)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 21))
		))

		(if (i32.ne (call $i32_align_switch (i32.const 4) (i32.const 4)) (i32.const 10)) (then     
			(call $proc_exit (i32.const 22))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 0) (i32.const 0)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 24))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 0) (i32.const 1)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 25))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 1) (i32.const 0)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 26))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 1) (i32.const 1)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 27))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 2) (i32.const 0)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 28))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 2) (i32.const 1)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 29))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 2) (i32.const 2)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 30))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 3) (i32.const 0)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 31))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 3) (i32.const 1)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 32))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 3) (i32.const 2)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 33))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 4) (i32.const 0)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 34))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 4) (i32.const 1)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 35))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 4) (i32.const 2)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 36))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 4) (i32.const 4)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 37))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 5) (i32.const 0)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 38))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 5) (i32.const 1)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 39))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 5) (i32.const 2)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 40))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 5) (i32.const 4)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 41))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 6) (i32.const 0)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 42))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 6) (i32.const 1)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 43))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 6) (i32.const 2)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 44))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 6) (i32.const 4)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 45))
		))

		(if (i64.ne (call $i64_align_switch (i32.const 6) (i32.const 8)) (i64.const 10)) (then     
			(call $proc_exit (i32.const 46))
		))

		(call $proc_exit (i32.const 0))
	)
)

;; Test that an i64 store with 4-byte alignment that's 4 bytes out of bounds traps without storing anything

;; (module
;;   (memory 1)
;;   (func $store (export "store") (param i32 i64)
;; 	(i64.store align=4 (local.get 0) (local.get 1))
;;   )
;;   (func $load (export "load") (param i32) (result i32)
;; 	(i32.load (local.get 0))
;;   )
;; )

;; (assert_trap (invoke "store" (i32.const 65532) (i64.const -1)) "out of bounds memory access")
;; ;; No memory was changed
;; (assert_return (invoke "load" (i32.const 65532)) (i32.const 0))
