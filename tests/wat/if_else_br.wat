;; aWsm adds implicit branches to if/else blocks if branches are not already present
;; This module tests to make sure that if and else blocks build both when they contain
;; a terminator and when they do not. All functions are exported to prevent dead-code
;; elimination.

(module
    (memory 1)

    (func (export "_start") (result i32)
        block
            i32.const 1
            i32.const 0
            i32.eq
            if
                br 0
                nop
            else
                br 1
                nop
            end ;; 0 breaks here
            i32.const 1
            return
        end ;; 1 breaks here
        i32.const 0
        return
    )
    (func (export "nop_nop") (result i32)
        block
            i32.const 1
            i32.const 0
            i32.eq
            if
                nop
            else
                nop
            end ;; 0 breaks here
            i32.const 1
            return
        end ;; 1 breaks here
        i32.const 0
        return
    )
    (func (export "br_nop") (result i32)
        block
            i32.const 1
            i32.const 0
            i32.eq
            if
                br 0
            else
                nop
            end ;; 0 breaks here
            i32.const 1
            return
        end ;; 1 breaks here
        i32.const 0
        return
    )
    (func (export "nop_br") (result i32)
        block
            i32.const 1
            i32.const 0
            i32.eq
            if
                br 0
                nop
            else
                br 1
                nop
            end ;; 0 breaks here
            i32.const 1
            return
        end ;; 1 breaks here
        i32.const 0
        return
    )
)
