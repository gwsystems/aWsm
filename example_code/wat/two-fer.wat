(module
  (memory (export "mem") 1)
  ;; Passive Data Segments
  (data $oneFor "One for ")
  (data $you "you")
  (data $oneForMe ", one for me.")
  ;; Length of Passive Data Segments
  (global $oneForSize i32 (i32.const 8))
  (global $youSize i32 (i32.const 3))
  (global $oneForMeSize i32 (i32.const 13))
  (func (export "twoFer") (param $offset i32) (param $length i32) (result i32 i32)
    (local $resultOffset i32)
    (local $resultLength i32)
    
    (local.set $resultOffset (i32.const 256))
    (local.set $resultLength (i32.const 0))
    
    (memory.init $oneFor (i32.add (local.get $resultOffset) (local.get $resultLength)) (i32.const 0) (global.get $oneForSize))
    (local.set $resultLength (i32.add (local.get $resultLength) (global.get $oneForSize)))
    
    (if (i32.eqz (local.get $length)) (then
      (memory.init $you (i32.add (local.get $resultOffset) (local.get $resultLength)) (i32.const 0) (global.get $youSize))
      (local.set $resultLength (i32.add (local.get $resultLength) (global.get $youSize)))
    ) (else 
      (memory.copy (i32.add (local.get $resultOffset) (local.get $resultLength)) (local.get $offset) (local.get $length))
      (local.set $resultLength (i32.add (local.get $resultLength) (local.get $length)))
    ))
    (memory.init $oneForMe (i32.add (local.get $resultOffset) (local.get $resultLength)) (i32.const 0) (global.get $oneForMeSize))
    (local.set $resultLength (i32.add (local.get $resultLength) (global.get $oneForMeSize)))
    
    (local.get $resultOffset)
    (local.get $resultLength)
  )
)
