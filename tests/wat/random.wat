(module
    (import "wasi_snapshot_preview1" "fd_write" (func $fd_write (param i32 i32 i32 i32) (result i32)))
    (import "wasi_snapshot_preview1" "random_get" (func $random_get (param i32 i32) (result i32)))
    (import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
    
    (memory 10)
    (export "memory" (memory 0))

    (global $stdout i32 (i32.const 1))

    ;; newline string
    (data (i32.const 6) "\n")
    (global $newline i32 (i32.const 6))

    ;; Buffer space set prior to calling fd_write
    
    (global $iov_base i32 (i32.const 8))
    (global $iov_len i32 (i32.const 12))
    (global $iov2_base i32 (i32.const 16))
    (global $iov2_len i32 (i32.const 20))
    (global $nchars i32 (i32.const 24))

    (table $tbl 0 anyfunc)

    ;; We also need to manually stub out this function
    (func (export "__wasm_call_ctors"))

    (func $_start (export "_start")
        ;; Setting second iov in vector to newline
        (i32.store (global.get $iov2_base) (global.get $newline))
        (i32.store (global.get $iov2_len) (i32.const 2))

        ;; Get 5 random bytes and write to stdout
        (call $random_get (i32.const 0) (i32.const 5))
        drop
        (i32.store (global.get $iov_base) (i32.const 0))  ;; iov.iov_base 
        (i32.store (global.get $iov_len) (i32.const 5))  ;; iov.iov_len 
        (call $fd_write
            (global.get $stdout) ;; target
            (global.get $iov_base) ;; start of iovs vector
            (i32.const 2) ;; size of iovs vector
            (global.get $nchars) ;; nwritten - A place in memory to store the number of bytes written
        )
        drop

        ;; Get 1000 random bytes and write to stdout
        (call $random_get (i32.const 100) (i32.const 100))
        drop
        (i32.store (global.get $iov_base) (i32.const 100))
        (i32.store (global.get $iov_len) (i32.const 100))
        (call $fd_write
            (global.get $stdout) ;; target
            (global.get $iov_base) ;; start of iovs vector
            (i32.const 2) ;; size of iovs vector
            (global.get $nchars) ;; nwritten
        )
        drop

        (call $proc_exit (i32.const 0))
    )

    ;; awsm uses main as the entrypoint, not the normal WASI _start
    (func (export "main")
        (call $_start)
    )
)
