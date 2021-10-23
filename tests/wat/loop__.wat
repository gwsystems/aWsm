;; Derived from https://github.com/WebAssembly/testsuite/blob/main/loop.wast

(module $loop-unit-tests
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))

    (memory 1)
    (export "memory" (memory 0))
    ;; (table $tbl 0 anyfunc)

    (func $dummy)

    (func $empty
        (loop)
        (loop $l)
    )

    (func $singular (result i32)
        (loop (nop))
        (loop (result i32) (i32.const 7))
    )

    ;; wat2wasm error: multiple loop results not currently supported.
    ;; (func $multi (result i32)
    ;;     (loop (call $dummy) (call $dummy) (call $dummy) (call $dummy))
    ;;     (loop (result i32) (call $dummy) (call $dummy) (i32.const 8) (call $dummy))
    ;;     (drop)
    ;;     (loop (result i32 i64 i32)
    ;;         (call $dummy) (call $dummy) (call $dummy) (i32.const 8) (call $dummy)
    ;;         (call $dummy) (call $dummy) (call $dummy) (i64.const 7) (call $dummy)
    ;;         (call $dummy) (call $dummy) (call $dummy) (i32.const 9) (call $dummy)
    ;;     )
    ;;     (drop) (drop)
    ;; )

    (func $nested (result i32)
        (loop (result i32)
            (loop (call $dummy) (block) (nop))
            (loop (result i32) (call $dummy) (i32.const 9))
        )
    )

    (func $deep (result i32)
        (loop (result i32) (block (result i32)
            (loop (result i32) (block (result i32)
                (loop (result i32) (block (result i32)
                    (loop (result i32) (block (result i32)
                        (loop (result i32) (block (result i32)
                            (loop (result i32) (block (result i32)
                                (loop (result i32) (block (result i32)
                                    (loop (result i32) (block (result i32)
                                        (loop (result i32) (block (result i32)
                                            (loop (result i32) (block (result i32)
                                                (loop (result i32) (block (result i32)
                                                    (loop (result i32) (block (result i32)
                                                        (loop (result i32) (block (result i32)
                                                            (loop (result i32) (block (result i32)
                                                                (loop (result i32) (block (result i32)
                                                                    (loop (result i32) (block (result i32)
                                                                        (loop (result i32) (block (result i32)
                                                                            (loop (result i32) (block (result i32)
                                                                                (loop (result i32) (block (result i32)
                                                                                    (loop (result i32) (block (result i32)
                                                                                        (call $dummy) (i32.const 150)
                                                                                    ))
                                                                                ))
                                                                            ))
                                                                        ))
                                                                    ))
                                                                ))
                                                            ))
                                                        ))
                                                    ))
                                                ))
                                            ))
                                        ))
                                    ))
                                ))
                            ))
                        ))
                    ))
                ))
            ))
        ))
    )

    (func $as-select-first (result i32)
        (select (loop (result i32) (i32.const 1)) (i32.const 2) (i32.const 3))
    )

    (func $as-select-mid (result i32)
        (select (i32.const 2) (loop (result i32) (i32.const 1)) (i32.const 3))
    )

    (func $as-select-last (result i32)
        (select (i32.const 2) (i32.const 3) (loop (result i32) (i32.const 1)))
    )

    (func $as-if-condition
        (loop (result i32) (i32.const 1)) (if (then (call $dummy)))
    )

    (func $as-if-then (result i32)
        (if (result i32) (i32.const 1) (then (loop (result i32) (i32.const 1))) (else (i32.const 2)))
    )
    (func $as-if-else (result i32)
        (if (result i32) (i32.const 1) (then (i32.const 2)) (else (loop (result i32) (i32.const 1))))
    )

    (func $as-br_if-first (result i32)
        (block (result i32) (br_if 0 (loop (result i32) (i32.const 1)) (i32.const 2)))
    )

    (func $as-br_if-last (result i32)
        (block (result i32) (br_if 0 (i32.const 2) (loop (result i32) (i32.const 1))))
    )

    (func $as-br_table-first (result i32)
        (block (result i32) (loop (result i32) (i32.const 1)) (i32.const 2) (br_table 0 0))
    )

    (func $as-br_table-last (result i32)
        (block (result i32) (i32.const 2) (loop (result i32) (i32.const 1)) (br_table 0 0))
    )

    (func $func (param i32 i32) (result i32) (local.get 0))
    
    (type $check (func (param i32 i32) (result i32)))
    
    (table funcref (elem $func))

    (func $as-call_indirect-first (result i32)
        (block (result i32)
            (call_indirect (type $check)
                (loop (result i32) (i32.const 1)) (i32.const 2) (i32.const 0)
            )
        )
    )

    (func $as-call_indirect-mid (result i32)
        (block (result i32)
            (call_indirect (type $check)
                (i32.const 2) (loop (result i32) (i32.const 1)) (i32.const 0)
            )
        )
    )
    
    (func $as-call_indirect-last (result i32)
        (block (result i32)
            (call_indirect (type $check)
                (i32.const 1) (i32.const 2) (loop (result i32) (i32.const 0))
            )
        )
    )

    (func $as-store-first
        (loop (result i32) (i32.const 1)) (i32.const 1) (i32.store)
    )
    (func $as-store-last
        (i32.const 10) (loop (result i32) (i32.const 1)) (i32.store)
    )

    (func $as-memory.grow-value (result i32)
        (memory.grow (loop (result i32) (i32.const 1)))
    )

    (func $f (param i32) (result i32) (local.get 0))

    (func $as-call-value (result i32)
        (call $f (loop (result i32) (i32.const 1)))
    )

    (func $as-return-value (result i32)
        (loop (result i32) (i32.const 1)) (return)
    )

    (func $as-drop-operand
        (drop (loop (result i32) (i32.const 1)))
    )

    (func $as-br-value (result i32)
        (block (result i32) (br 0 (loop (result i32) (i32.const 1))))
    )

    (func $as-local.set-value (result i32)
        (local i32) (local.set 0 (loop (result i32) (i32.const 1))) (local.get 0)
    )

    (func $as-local.tee-value (result i32)
        (local i32) (local.tee 0 (loop (result i32) (i32.const 1)))
    )

    (global $a (mut i32) (i32.const 0))
    
    (func $as-global.set-value (result i32)
        (global.set $a (loop (result i32) (i32.const 1)))
        (global.get $a)
    )

    (func $as-load-operand (result i32)
        (i32.load (loop (result i32) (i32.const 1)))
    )

    (func $as-unary-operand (result i32)
        (i32.ctz 
            (loop (result i32) 
                (call $dummy) 
                (i32.const 13)
            )
        )
    )

    (func $as-binary-operand (result i32)
        (i32.mul
            (loop (result i32) 
                (call $dummy) 
                (i32.const 3)
            )
            (loop (result i32) 
                (call $dummy) 
                (i32.const 4)
            )
        )
    )
    
    (func $as-test-operand (result i32)
        (i32.eqz (loop (result i32) (call $dummy) (i32.const 13)))
    )

    (func $as-compare-operand (result i32)
        (f32.gt
            (loop (result f32) (call $dummy) (f32.const 3))
            (loop (result f32) (call $dummy) (f32.const 3))
        )
    )

    ;; wat2wasm error: multiple loop results not currently supported.
    ;; (func $as-binary-operands (result i32)
    ;;     (i32.mul
    ;;         (loop (result i32 i32)
    ;;         (call $dummy) (i32.const 3) (call $dummy) (i32.const 4)
    ;;         )
    ;;     )
    ;; )

    ;; wat2wasm error: multiple loop results not currently supported.
    ;; (func $as-compare-operands (result i32)
    ;;     (f32.gt
    ;;         (loop (result f32 f32)
    ;;         (call $dummy) (f32.const 3) (call $dummy) (f32.const 3)
    ;;         )
    ;;     )
    ;; )

    ;; wat2wasm error: multiple loop results not currently supported.
    ;; (func $as-mixed-operands (result i32)
    ;;     (loop (result i32 i32)
    ;;         (call $dummy) (i32.const 3) (call $dummy) (i32.const 4)
    ;;     )
    ;;     (i32.const 5)
    ;;     (i32.add)
    ;;     (i32.mul)
    ;; )

    (func $break-bare (result i32)
        (block (loop (br 1) (br 0) (unreachable)))
        (block (loop (br_if 1 (i32.const 1)) (unreachable)))
        (block (loop (br_table 1 (i32.const 0)) (unreachable)))
        (block (loop (br_table 1 1 1 (i32.const 1)) (unreachable)))
        (i32.const 19)
    )

    ;; wat2wasm error: loop params not currently supported.
    ;; (func $break-value (result i32)
    ;;     (block (result i32)
    ;;         (i32.const 0)
    ;;         (loop (param i32)
    ;;             (block (br 2 (i32.const 18)))
    ;;             (br 0 (i32.const 20))
    ;;         )
    ;;         (i32.const 19)
    ;;     )
    ;; )

    ;; wat2wasm error: loop params not currently supported.
    ;; (func $break-multi-value (result i32 i32 i64)
    ;;     (block (result i32 i32 i64)
    ;;         (i32.const 0) (i32.const 0) (i64.const 0)
    ;;         (loop (param i32 i32 i64)
    ;;             (block (br 2 (i32.const 18) (i32.const -18) (i64.const 18)))
    ;;             (br 0 (i32.const 20) (i32.const -20) (i64.const 20))
    ;;         )
    ;;         (i32.const 19) (i32.const -19) (i64.const 19)
    ;;     )
    ;; )
    
    (func $break-repeated (result i32)
        (block (result i32)
            (loop (result i32)
                (br 1 (i32.const 18))
                (br 1 (i32.const 19))
                (drop (br_if 1 (i32.const 20) (i32.const 0)))
                (drop (br_if 1 (i32.const 20) (i32.const 1)))
                (br 1 (i32.const 21))
                (br_table 1 (i32.const 22) (i32.const 0))
                (br_table 1 1 1 (i32.const 23) (i32.const 1))
                (i32.const 21)
            )
        )
    )

    ;; TODO: Revisit this test! This causes malformed stacks
    ;; (func $break-inner (result i32)
    ;;     (local i32)
    ;;     (local.set 0 (i32.const 0))
    ;;     (local.set 0 (i32.add (local.get 0) (block (result i32) (loop (result i32) (block (result i32) (br 2 (i32.const 0x1)))))))
    ;;     (local.set 0 (i32.add (local.get 0) (block (result i32) (loop (result i32) (loop (result i32) (br 2 (i32.const 0x2)))))))
    ;;     (local.set 0 (i32.add (local.get 0) (block (result i32) (loop (result i32) (block (result i32) (loop (result i32) (br 1 (i32.const 0x4))))))))
    ;;     (local.set 0 (i32.add (local.get 0) (block (result i32) (loop (result i32) (i32.ctz (br 1 (i32.const 0x8)))))))
    ;;     (local.set 0 (i32.add (local.get 0) (block (result i32) (loop (result i32) (i32.ctz (loop (result i32) (br 2 (i32.const 0x10))))))))
    ;;     (local.get 0)
    ;; )

    ;; TODO: Revisit this test! This causes malformed stacks
    ;; (func $cont-inner (result i32)
    ;;     (local i32)
    ;;     (local.set 0 (i32.const 0))
    ;;     (local.set 0 (i32.add (local.get 0) (loop (result i32) (loop (result i32) (br 1)))))
    ;;     (local.set 0 (i32.add (local.get 0) (loop (result i32) (i32.ctz (br 0)))))
    ;;     (local.set 0 (i32.add (local.get 0) (loop (result i32) (i32.ctz (loop (result i32) (br 1))))))
    ;;     (local.get 0)
    ;; )

    ;; wat2wasm error: loop params not currently supported.
    ;; (func $param (result i32)
    ;;     (i32.const 1)
    ;;     (loop (param i32) (result i32)
    ;;         (i32.const 2)
    ;;         (i32.add)
    ;;     )
    ;; )

    ;; wat2wasm error: loop params not currently supported.
    ;; (func $params (result i32)
    ;;     (i32.const 1)
    ;;     (i32.const 2)
    ;;     (loop (param i32 i32) (result i32)
    ;;         (i32.add)
    ;;     )
    ;; )
    
    ;; wat2wasm error: multiple loop results not currently supported.
    ;; (func $params-id (result i32)
    ;;     (i32.const 1)
    ;;     (i32.const 2)
    ;;     (loop (param i32 i32) (result i32 i32))
    ;;     (i32.add)
    ;; )

    ;; wat2wasm error: loop params not currently supported.
    ;; (func $param-break (result i32)
    ;;     (local $x i32)
    ;;     (i32.const 1)
    ;;     (loop (param i32) (result i32)
    ;;         (i32.const 4)
    ;;         (i32.add)
    ;;         (local.tee $x)
    ;;         (local.get $x)
    ;;         (i32.const 10)
    ;;         (i32.lt_u)
    ;;         (br_if 0)
    ;;     )
    ;; )

    ;; wat2wasm error: loop params not currently supported.
    ;; (func $params-break (result i32)
    ;;     (local $x i32)
    ;;     (i32.const 1)
    ;;     (i32.const 2)
    ;;     (loop (param i32 i32) (result i32)
    ;;         (i32.add)
    ;;         (local.tee $x)
    ;;         (i32.const 3)
    ;;         (local.get $x)
    ;;         (i32.const 10)
    ;;         (i32.lt_u)
    ;;         (br_if 0)
    ;;         (drop)
    ;;     )
    ;; )

    ;; wat2wasm error: multiple loop results not currently supported.
    ;; (func $params-id-break (result i32)
    ;;     (local $x i32)
    ;;     (local.set $x (i32.const 0))
    ;;     (i32.const 1)
    ;;     (i32.const 2)
    ;;     (loop (param i32 i32) (result i32 i32)
    ;;         (local.set $x (i32.add (local.get $x) (i32.const 1)))
    ;;         (br_if 0 (i32.lt_u (local.get $x) (i32.const 10)))
    ;;     )
    ;;     (i32.add)
    ;; )

    (func $effects (result i32)
        (local i32)
        (block
            (loop
                (local.set 0 (i32.const 1))
                (local.set 0 (i32.mul (local.get 0) (i32.const 3)))
                (local.set 0 (i32.sub (local.get 0) (i32.const 5)))
                (local.set 0 (i32.mul (local.get 0) (i32.const 7)))
                (br 1)
                (local.set 0 (i32.mul (local.get 0) (i32.const 100)))
            )
        )
        (i32.eq (local.get 0) (i32.const -14))
    )

    (func $while (param i64) (result i64)
        (local i64)
        (local.set 1 (i64.const 1))
        (block
            (loop
                (br_if 1 (i64.eqz (local.get 0)))
                (local.set 1 (i64.mul (local.get 0) (local.get 1)))
                (local.set 0 (i64.sub (local.get 0) (i64.const 1)))
                (br 0)
            )
        )
        (local.get 1)
    )

    (func $for (param i64) (result i64)
        (local i64 i64)
        (local.set 1 (i64.const 1))
        (local.set 2 (i64.const 2))
        (block
            (loop
                (br_if 1 (i64.gt_u (local.get 2) (local.get 0)))
                (local.set 1 (i64.mul (local.get 1) (local.get 2)))
                (local.set 2 (i64.add (local.get 2) (i64.const 1)))
                (br 0)
            )
        )
        (local.get 1)
    )

    (func $nesting (param f32 f32) (result f32)
        (local f32 f32)
        (block
            (loop
                (br_if 1 (f32.eq (local.get 0) (f32.const 0)))
                (local.set 2 (local.get 1))
                (block
                (loop
                    (br_if 1 (f32.eq (local.get 2) (f32.const 0)))
                    (br_if 3 (f32.lt (local.get 2) (f32.const 0)))
                    (local.set 3 (f32.add (local.get 3) (local.get 2)))
                    (local.set 2 (f32.sub (local.get 2) (f32.const 2)))
                    (br 0)
                )
                )
                (local.set 3 (f32.div (local.get 3) (local.get 0)))
                (local.set 0 (f32.sub (local.get 0) (f32.const 1)))
                (br 0)
            )
        )
        (local.get 3)
    )

    (type $block-sig-1 (func))
    (type $block-sig-2 (func (result i32)))
    (type $block-sig-3 (func (param $x i32)))
    (type $block-sig-4 (func (param i32 f64 i32) (result i32 f64 i32)))

    ;; wat2wasm error: multiple loop results not currently supported.
    ;; (func $type-use
    ;;     (loop (type $block-sig-1))
    ;;     (loop (type $block-sig-2) (i32.const 0))
    ;;     (loop (type $block-sig-3) (drop))
    ;;     (i32.const 0) (f64.const 0) (i32.const 0)
    ;;     (loop (type $block-sig-4))
    ;;     (drop) (drop) (drop)
    ;;     (loop (type $block-sig-2) (result i32) (i32.const 0))
    ;;     (loop (type $block-sig-3) (param i32) (drop))
    ;;     (i32.const 0) (f64.const 0) (i32.const 0)
    ;;     (loop (type $block-sig-4)
    ;;     (param i32) (param f64 i32) (result i32 f64) (result i32)
    ;;     )
    ;;     (drop) (drop) (drop)
    ;; )

	(func (export "_start") (param i32)

        (call $empty)

        (call $singular (i32.const 7))
        drop
        drop

        ;; Disabled because this requires multi-value support
        ;; (call $multi (i32.const 8))

        (call $nested (i32.const 9))
        drop
        drop

        (call $deep (i32.const 150))
        drop
        drop

        (call $as-select-first (i32.const 1))
        drop
        drop

        (call $as-select-mid (i32.const 2))
        drop
        drop
        
        (call $as-select-last (i32.const 2))
        drop
        drop

        (call $as-if-condition)
        
        (call $as-if-then (i32.const 1))
        drop
        drop

        (call $as-if-else (i32.const 2))
        drop
        drop

        (call $as-br_if-first (i32.const 1))
        drop
        drop

        (call $as-br_if-last (i32.const 2))
        drop
        drop

        (call $as-br_table-first (i32.const 1))
        drop
        drop

        (call $as-br_table-last (i32.const 2))
        drop
        drop

        (call $as-call_indirect-first (i32.const 1))
        drop
        drop

        (call $as-call_indirect-mid (i32.const 2))
        drop
        drop

        (call $as-call_indirect-last (i32.const 1))
        drop
        drop

        (call $as-store-first)

        (call $as-store-last)

        (call $as-memory.grow-value (i32.const 1))
        drop
        drop

        (call $as-call-value (i32.const 1))
        drop
        drop
        
        (call $as-return-value (i32.const 1))
        drop
        drop

        (call $as-drop-operand)

        (call $as-br-value (i32.const 1))
        drop
        drop

        (call $as-local.set-value (i32.const 1))
        drop
        drop

        (call $as-local.tee-value (i32.const 1))
        drop
        drop
        
        (call $as-global.set-value (i32.const 1))
        drop
        drop

        (call $as-load-operand (i32.const 1))
        drop
        drop

        (call $as-unary-operand (i32.const 0))
        drop
        drop

        (call $as-binary-operand (i32.const 12))
        drop
        drop

        (call $as-test-operand (i32.const 0))
        drop
        drop

        (call $as-compare-operand (i32.const 0))
        drop
        drop
        
        ;; (call $as-binary-operands (i32.const 12))
        ;; (call $as-compare-operands (i32.const 0))
        ;; (call $as-mixed-operands (i32.const 27))

        (call $break-bare (i32.const 19))
        drop
        drop

        ;; (call $break-value (i32.const 18))
        ;; (call $break-multi-value (i32.const 18) (i32.const -18) (i64.const 18))
        (call $break-repeated (i32.const 18))
        drop
        drop

        ;; (call $break-inner (i32.const 0x1f))

        ;; (call $param (i32.const 3))
        ;; (call $params (i32.const 3))
        ;; (call $params-id (i32.const 3))
        ;; (call $param-break (i32.const 13))
        ;; (call $params-break (i32.const 12))
        ;; (call $params-id-break (i32.const 3))

        (call $effects (i32.const 1))
        drop
        drop

        (if (i64.ne (call $while (i64.const 0)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 1))
		))

        (if (i64.ne (call $while (i64.const 1)) (i64.const 1)) (then     
			(call $proc_exit (i32.const 2))
		))

        (if (i64.ne (call $while (i64.const 2)) (i64.const 2))(then     
			(call $proc_exit (i32.const 3))
		))

        (if (i64.ne (call $while (i64.const 3)) (i64.const 6))(then     
			(call $proc_exit (i32.const 4))
		))
        
        (if (i64.ne (call $while (i64.const 5)) (i64.const 120))(then     
			(call $proc_exit (i32.const 5))
		))

        (if (i64.ne (call $while (i64.const 20)) (i64.const 2432902008176640000))(then     
			(call $proc_exit (i32.const 6))
		))

        (if (i64.ne (call $for (i64.const 0)) (i64.const 1))(then     
			(call $proc_exit (i32.const 7))
		))

        (if (i64.ne (call $for (i64.const 1)) (i64.const 1))(then     
			(call $proc_exit (i32.const 8))
		))
        
        (if (i64.ne (call $for (i64.const 2)) (i64.const 2))(then     
			(call $proc_exit (i32.const 9))
		))

        (if (i64.ne (call $for (i64.const 3)) (i64.const 6))(then     
			(call $proc_exit (i32.const 10))
		))

        (if (i64.ne (call $for (i64.const 5)) (i64.const 120))(then     
			(call $proc_exit (i32.const 11))
		))

        (if (i64.ne (call $for (i64.const 20)) (i64.const 2432902008176640000))(then     
			(call $proc_exit (i32.const 12))
		))

        (if (f32.ne (call $nesting (f32.const 0) (f32.const 7)) (f32.const 0))(then     
			(call $proc_exit (i32.const 13))
		))

        (if (f32.ne (call $nesting (f32.const 7) (f32.const 0)) (f32.const 0))(then     
			(call $proc_exit (i32.const 14))
		))

        (if (f32.ne (call $nesting (f32.const 1) (f32.const 1)) (f32.const 1))(then     
			(call $proc_exit (i32.const 15))
		))

        (if (f32.ne (call $nesting (f32.const 1) (f32.const 2)) (f32.const 2))(then     
			(call $proc_exit (i32.const 16))
		))

        (if (f32.ne (call $nesting (f32.const 1) (f32.const 3)) (f32.const 4))(then     
			(call $proc_exit (i32.const 17))
		))

        (if (f32.ne (call $nesting (f32.const 1) (f32.const 4)) (f32.const 6))(then     
			(call $proc_exit (i32.const 18))
		))

        (if (f32.ne (call $nesting (f32.const 1) (f32.const 100)) (f32.const 2550))(then     
			(call $proc_exit (i32.const 19))
		))

        (if (f32.ne (call $nesting (f32.const 1) (f32.const 101)) (f32.const 2601))(then     
			(call $proc_exit (i32.const 20))
		))

        (if (f32.ne (call $nesting (f32.const 2) (f32.const 1)) (f32.const 1))(then     
			(call $proc_exit (i32.const 21))
		))

        (if (f32.ne (call $nesting (f32.const 3) (f32.const 1)) (f32.const 1))(then     
			(call $proc_exit (i32.const 22))
		))
        
        (if (f32.ne (call $nesting (f32.const 10) (f32.const 1)) (f32.const 1))(then     
			(call $proc_exit (i32.const 23))
		))
        
        (if (f32.ne (call $nesting (f32.const 2) (f32.const 2)) (f32.const 3))(then     
			(call $proc_exit (i32.const 24))
		))
        
        (if (f32.ne (call $nesting (f32.const 2) (f32.const 3)) (f32.const 4))(then     
			(call $proc_exit (i32.const 25))
		))
        
        (if (f32.ne (call $nesting (f32.const 7) (f32.const 4)) (f32.const 10.3095235825))(then     
			(call $proc_exit (i32.const 26))
		))
        
        (if (f32.ne (call $nesting (f32.const 7) (f32.const 100)) (f32.const 4381.54785156))(then     
			(call $proc_exit (i32.const 27))
		))
        
        (if (f32.ne (call $nesting (f32.const 7) (f32.const 101)) (f32.const 2601))(then     
			(call $proc_exit (i32.const 28))
		))
        
        
		(call $proc_exit (i32.const 0))
	)
)
