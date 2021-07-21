(module
    (import "wasi_snapshot_preview1" "fd_write" (func $fd_write (param i32 i32 i32 i32) (result i32)))
    (import "wasi_snapshot_preview1" "random_get" (func $random_get (param i32 i32) (result i32)))
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    
    (memory 1)
    (export "memory" (memory 0))

    (table $tbl 0 anyfunc)

    ;; We also need to manually stub out this function
    (func (export "__wasm_call_ctors"))

    (func $panic
        (i32.store (i32.const 0) (i32.const 8))  ;; iov.iov_base 
        (i32.store (i32.const 4) (i32.const 6))  ;; iov.iov_len 
        (call $fd_write
            (i32.const 1) ;; file_descriptor - 1 for stdout
            (i32.const 0) ;; *iovs - The pointer to the iov array, which is stored at memory location 0
            (i32.const 1) ;; iovs_len - We're printing 1 string stored in an iov - so one.
            (i32.const 20) ;; nwritten - A place in memory to store the number of bytes written
        )
        drop ;; Discard the number of bytes written from the top of the stack
        (call $proc_exit (i32.const 1))
    )
    ;; Returns the number of passed tests
    (func $_start (export "_start")
        (call $random_get (i32.const 0) (i32.const 5))
        drop

        (i32.store (i32.const 8) (i32.const 0))  ;; iov.iov_base 
        (i32.store (i32.const 12) (i32.const 5))  ;; iov.iov_len 

        (call $fd_write
            (i32.const 1) ;; file_descriptor - 1 for stdout
            (i32.const 8) ;; *iovs - The pointer to the iov array, which is stored at memory location 0
            (i32.const 1) ;; iovs_len - We're printing 1 string stored in an iov - so one.
            (i32.const 20) ;; nwritten - A place in memory to store the number of bytes written
        )
        drop

        (call $proc_exit (i32.const 0))
    )

    ;; awsm uses main as the entrypoint, not the normal WASI _start
    (func (export "main")
        (call $_start)
    )
)
