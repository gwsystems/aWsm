(module
  (memory (export "mem") 1)
  (func (export "toRna") (param $offset i32) (param $length i32) (result i32 i32)
    (local $i i32)
    (local $charCode i32)
    (local $cursor i32)
    (local.set $i (i32.const 0))
    (if (i32.lt_u (local.get $i) (local.get $length)) (then 
      (loop
        (local.set $cursor (i32.add (local.get $offset) (local.get $i)))
        (local.set $charCode (i32.load8_u (local.get $cursor)))
        ;; G -> C
        (if (i32.eq (local.get $charCode) (i32.const 67)) (then
          (i32.store8 (local.get $cursor) (i32.const 71)))
        ;; C -> G
        (else (if (i32.eq (local.get $charCode) (i32.const 71)) (then
          (i32.store8 (local.get $cursor) (i32.const 67)))
        ;; T -> A
        (else (if (i32.eq (local.get $charCode) (i32.const 84)) (then
          (i32.store8 (local.get $cursor) (i32.const 65)))
        ;; A -> U
        (else (if (i32.eq (local.get $charCode) (i32.const 65)) (then
          (i32.store8 (local.get $cursor) (i32.const 85)))
        )))))))
        (br_if 0 (i32.lt_u (local.tee $i (i32.add (local.get $i) (i32.const 1))) (local.get $length)))
      )
    ))
    (return (local.get $offset) (local.get $length))
  )
)
