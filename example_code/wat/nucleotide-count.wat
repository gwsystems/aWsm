(module
  (memory (export "mem") 1)
  (func (export "countNucleotides") (param $offset i32) (param $length i32) (result i32 i32 i32 i32)
    (local $i i32)
    (local $aCount i32)
    (local $cCount i32)
    (local $gCount i32)
    (local $tCount i32)
    (local $charCode i32)
    (local.set $i (i32.const 0))
    (local.set $aCount (i32.const 0))
    (local.set $cCount (i32.const 0))
    (local.set $gCount (i32.const 0))
    (local.set $tCount (i32.const 0))
    (if (i32.lt_u (local.get $i) (local.get $length)) (then
      (loop
        (local.set $charCode (i32.load8_u (i32.add (local.get $offset) (local.get $i))))
        (if (i32.eq (local.get $charCode) (i32.const 65)) (then
          (local.set $aCount (i32.add (local.get $aCount) (i32.const 1)))          
        ) (else (if (i32.eq (local.get $charCode) (i32.const 67)) (then
          (local.set $cCount (i32.add (local.get $cCount) (i32.const 1)))          
        ) (else (if (i32.eq (local.get $charCode) (i32.const 71)) (then
          (local.set $gCount (i32.add (local.get $gCount) (i32.const 1)))          
        ) (else (if (i32.eq (local.get $charCode) (i32.const 84)) (then
          (local.set $tCount (i32.add (local.get $tCount) (i32.const 1)))          
        ) (else 
          (return 
            (i32.const -1)
            (i32.const -1)
            (i32.const -1)
            (i32.const -1)
          )
        ))))))))
        
        (br_if 0 (i32.lt_u (local.tee $i (i32.add (local.get $i) (i32.const 1))) (local.get $length)))
      )
    ))
    
    (return 
      (local.get $aCount)
      (local.get $cCount)
      (local.get $gCount)
      (local.get $tCount)
    )
  )
)
