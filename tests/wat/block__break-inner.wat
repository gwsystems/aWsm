(module
    (memory 1)
	(export "memory" (memory 0))

    (func $break-inner (export "break-inner") (result i32)
        (local i32)
        (local.set 0 (i32.const 0))
        (local.set 0 (i32.add (local.get 0) (block (result i32) (block (result i32) (br 1 (i32.const 0x1))))))
        (local.set 0 (i32.add (local.get 0) (block (result i32) (block (br 0)) (i32.const 0x2))))
        (local.set 0
            (i32.add (local.get 0) (block (result i32) (i32.ctz (br 0 (i32.const 0x4)))))
        )
        (local.set 0
            (i32.add (local.get 0) (block (result i32) (i32.ctz (block (result i32) (br 1 (i32.const 0x8))))))
        )
        (local.get 0)
    )

    (func (export "_start") (param i32)
        (call $break-inner (i32.const 0xf))
        drop
        drop
    )
)
