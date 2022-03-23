(module
  (memory (export "mem") 1)
 
  (func (export "reverseString") (param $offset i32) (param $length i32) (result i32 i32)
    (local $low i32)
    (local $high i32)
    (local $temp i32)
    (local.set $low (local.get $offset))
    (local.set $high (i32.sub (i32.add (local.get $offset) (local.get $length)) (i32.const 1)))
    (if (i32.lt_u (local.get $low) (local.get $high)) (then (loop
      (local.set $temp (i32.load8_u (local.get $high)))
      (i32.store8 (local.get $high) (i32.load8_u (local.get $low)))
      (i32.store8 (local.get $low) (local.get $temp))
      
      (br_if 0 (i32.lt_u 
        (local.tee $low (i32.add (local.get $low) (i32.const 1))) 
        (local.tee $high (i32.sub (local.get $high) (i32.const 1)))
    )))))
    
    (return (local.get $offset) (local.get $length))
  )
)
