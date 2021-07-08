
(module
    ;; We need to declare a linear memory or we assert out in Awsm
    (memory 1)

    ;; This doesn't seem to be needed, but adding just in case
    (export "memory" (memory 0))

    ;; We need a function table or we assert out in Awsm
    (table $tbl 0 anyfunc)

    ;; We also need to manually stub out this function
    (func (export "__wasm_call_ctors"))

    ;; Returns the number of passed tests
    (func (export "main")
        (result i32)
        (local $tests_passed i32)
        (local $n i32)

        (local.set $tests_passed (i32.const 0))

        ;; Test 1: if executes then block when condition evaluates to true
        (local.set $n (i32.const 1))
        (if (i32.eq (local.get $n) (i32.const 1)) (then 
            (local.set $tests_passed (i32.add (local.get $tests_passed) (i32.const 1)))
        ) (else 
            unreachable
        ))

        ;; Test 2: if executes else block when condition evaluates to false
        (local.set $n (i32.const 0))
        (if (i32.eq (local.get $n) (i32.const 1)) (then 
            unreachable
        ) (else 
            (local.set $tests_passed (i32.add (local.get $tests_passed) (i32.const 1)))
        ))

        ;; validate that tests_passed was incremented the expected number
        (if (i32.ne (local.get $tests_passed) (i32.const 2)) (then 
            unreachable
        ))
        
        (return (local.get $tests_passed))
    )
)