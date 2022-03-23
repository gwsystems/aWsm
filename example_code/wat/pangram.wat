(module
  (memory (export "mem") 1)
  ;; checks if charCode is a letter [a-zA-Z]
  (func $isalpha (param $charCode i32) (result i32)
    (i32.or
      (call $islower (local.get $charCode))
      (call $isupper (local.get $charCode))
    )
  )
  ;; checks if charCode is a lowercase letter [a-z]
  (func $islower (param $charCode i32) (result i32)
    (i32.and
      (i32.ge_s (local.get $charCode) (i32.const  97))
      (i32.le_s (local.get $charCode) (i32.const  122))
    )
  )
  ;; checks if charCode is an uppercase letter [A-Z]
  (func $isupper (param $charCode i32) (result i32)
    (i32.and
      (i32.ge_s (local.get $charCode) (i32.const 65))
      (i32.le_s (local.get $charCode) (i32.const 90))
    )
  )
  ;; Converts lowercase ASCII chars to uppercase equivalents
  (func $toupper (param $charCode i32) (result i32)
    (if (call $islower (local.get $charCode)) (then
      ;; Distance between A and a is 32
      (return (i32.sub (local.get $charCode) (i32.const 32)))
    ))
    (return (local.get $charCode))
  )
  ;; return 1 if pangram, 0 otherwise
  (func (export "isPangram") (param $offset i32) (param $length i32) (result i32)
    (local $i i32)
    (local $charCode i32)
    (local $bagCursor i32)
    (local.set $i (i32.const 0))
    
    ;; Initialize bag of u8 sized to the 26 ASCII characters
    (memory.fill (i32.const 512) (i32.const 0) (i32.const 26))
    (if (i32.lt_s (local.get $i) (local.get $length)) (then
      (loop
        (local.set $charCode (i32.load8_u (i32.add (local.get $offset) (local.get $i))))
        
        (if (call $isalpha (local.get $charCode)) (then
          (local.set $bagCursor (i32.add (i32.const 512) (i32.sub (call $toupper (local.get $charCode)) (i32.const 65))))
          (i32.store8 
            (local.get $bagCursor)
            (i32.add (i32.load8_u (local.get $bagCursor)) (i32.const 1))
          )
        ))
        (br_if 0 (i32.lt_s (local.tee $i (i32.add (local.get $i) (i32.const 1))) (local.get $length)))
      )
    ))
    (local.set $i (i32.const 0))
    (if (i32.lt_s (local.get $i) (i32.const 26)) (then
      (loop
        (local.set $bagCursor (i32.add (i32.const 512) (local.get $i)))
        (if (i32.eqz (i32.load8_u (local.get $bagCursor))) (then
          (return (i32.const 0))  
        ))
        (br_if 0 (i32.lt_s (local.tee $i (i32.add (local.get $i) (i32.const 1))) (i32.const 26)))
      )
    ))
       
    (i32.const 1)
  )
)
