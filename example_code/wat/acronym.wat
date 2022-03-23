(module
  (memory (export "mem") 1)
  ;; checks if charCode is a letter or digit [0-9a-zA-Z]
  (func $isalnum (param $charCode i32) (result i32)
    (i32.or 
      (call $isdigit (local.get $charCode))
      (call $isalpha (local.get $charCode))
    )
  )
  ;; checks if charCode is a letter [a-zA-Z]
  (func $isalpha (param $charCode i32) (result i32)
    (i32.or
      (call $islower (local.get $charCode))
      (call $isupper (local.get $charCode))
    )
  )
  ;; checks if charCode is a 7-bit unsigned char value that fits into the ASCII character set.
  (func $isascii (param $charCode i32) (result i32)
    (i32.and
      (i32.ge_s (local.get $charCode) (i32.const 0))
      (i32.le_s (local.get $charCode) (i32.const 127))
    )
  )
  ;; Checks if charCode is a non-printable control code
  ;; This includes the C0 control set (0-31) and DEL (127)
  (func $iscntrl (param $charCode i32) (result i32)
    (i32.or
      (i32.and
        (i32.ge_s (local.get $charCode) (i32.const  0))
        (i32.le_s (local.get $charCode) (i32.const 31))
      )
      (i32.eq (local.get $charCode) (i32.const 127))
    )
  )
  ;; checks if charCode is a digit [0-9]
  (func $isdigit (param $charCode i32) (result i32)
    (i32.and
      (i32.ge_s (local.get $charCode) (i32.const 48))
      (i32.le_s (local.get $charCode) (i32.const 57))
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
  ;; checks if charCode is a character that would be written to a graphical device
  (func $isgraph (param $charCode i32) (result i32)
    (i32.and 
      (call $isascii (local.get $charCode))
      (i32.and 
        (i32.eqz (call $iscntrl (local.get $charCode)))
        (i32.eqz (call $isspace (local.get $charCode)))
      )
    )
  )
  (func $ispunct (export "ispunct") (param $charCode i32) (result i32)
    (i32.and
      (call $isgraph (local.get $charCode))
      (i32.eqz (call $isalnum (local.get $charCode)))
    )
  )
  ;; checks if charCode is a space or one of the standard motion control characters.
  (func $isspace (export "isspace") (param $charCode i32) (result i32)
    (if (i32.eq (local.get $charCode) (i32.const  9)) (then (return (i32.const 1)))) ;; horizontal tab
    (if (i32.eq (local.get $charCode) (i32.const 10)) (then (return (i32.const 1)))) ;; line feed
    (if (i32.eq (local.get $charCode) (i32.const 11)) (then (return (i32.const 1)))) ;; vertical tab
    (if (i32.eq (local.get $charCode) (i32.const 12)) (then (return (i32.const 1)))) ;; form feed
    (if (i32.eq (local.get $charCode) (i32.const 13)) (then (return (i32.const 1)))) ;; carraige return
    (if (i32.eq (local.get $charCode) (i32.const 32)) (then (return (i32.const 1)))) ;; space
    (i32.const 0)
  )
  (func $toupper (param $charCode i32) (result i32)
    (if (call $islower (local.get $charCode)) (then
      ;; Distance between A and a is 32
      (return (i32.sub (local.get $charCode) (i32.const 32)))
    ))
    (return (local.get $charCode))
  )
  (func (export "parse") (param $offset i32) (param $length i32) (result i32 i32)
    (local $i i32)
    (local $currentCharCode i32)
    (local $lastCharCode i32)
    (local $resultLength i32)
    
    (local.set $i (i32.const 0))
    ;; Initialize last char code as a space so the first char is potentially used in the acronym
    (local.set $lastCharCode (i32.const 32))
    (local.set $resultLength (i32.const 0))
    (if (i32.lt_s (local.get $resultLength) (local.get $length)) (then
      (loop
        (local.set $currentCharCode (i32.load8_u (i32.add (local.get $offset) (local.get $i))))
        ;; Add alphabetic chars that follow spaces or punctuation other than an apostrophe, setting to uppercase
        (if (i32.and 
          (i32.or
              (call $isspace (local.get $lastCharCode))
              (i32.and
                (call $ispunct (local.get $lastCharCode))
                (i32.ne (local.get $lastCharCode) (i32.const 39))
              )
          )
          (call $isalpha (local.get $currentCharCode))
        ) (then 
          (i32.store8 (i32.add (local.get $offset)(local.get $resultLength)) (call $toupper (local.get $currentCharCode)))
          (local.set $resultLength (i32.add (local.get $resultLength) (i32.const 1)))
        ))
        (local.set $lastCharCode (local.get $currentCharCode))
        (br_if 0 (i32.lt_s (local.tee $i (i32.add (local.get $i) (i32.const 1))) (local.get $length)))
      )  
    ))
    
    (return (local.get $offset) (local.get $resultLength))
  )
)
