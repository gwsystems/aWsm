(module
  (import "console" "log_mem_as_utf8" (func $log_mem_as_utf8 (param $byteOffset i32) (param $length i32)))
  
  (memory (export "mem") 1)
  (data $colors "black,brown,red,orange,yellow,green,blue,violet,grey,white")
  
  (global $colorsOffset i32 (i32.const 100))
  (global $colorsLength i32 (i32.const  58))
  
  (global $blackOffset  i32 (i32.const   0))
  (global $blackLength  i32 (i32.const   5))
  (global $blackValue   i32 (i32.const   0))
  
  (global $brownOffset  i32 (i32.const   6))
  (global $brownLength  i32 (i32.const   5))
  (global $brownValue   i32 (i32.const   1))
  
  (global $redOffset    i32 (i32.const  12))
  (global $redLength    i32 (i32.const   3))
  (global $redValue     i32 (i32.const   2))
  
  (global $orangeOffset i32 (i32.const  16))
  (global $orangeLength i32 (i32.const   6))
  (global $orangeValue  i32 (i32.const   3))
  
  (global $yellowOffset i32 (i32.const  23))
  (global $yellowLength i32 (i32.const   6))
  (global $yellowValue  i32 (i32.const   4))
  
  (global $greenOffset  i32 (i32.const  30))
  (global $greenLength  i32 (i32.const   5))
  (global $greenValue   i32 (i32.const   5))
  
  (global $blueOffset   i32 (i32.const  36))
  (global $blueLength   i32 (i32.const   4))
  (global $blueValue    i32 (i32.const   6))
  
  (global $violetOffset i32 (i32.const  41))
  (global $violetLength i32 (i32.const   6))
  (global $violetValue  i32 (i32.const   7))
  
  (global $greyOffset   i32 (i32.const  48))
  (global $greyLength   i32 (i32.const   4))
  (global $greyValue    i32 (i32.const   8))
  
  (global $whiteOffset  i32 (i32.const  53))
  (global $whiteLength  i32 (i32.const   5))
  (global $whiteValue   i32 (i32.const   9))
  
  (start $initialize)
  
  ;; Called each time a module is initialized
  ;; Can be used to populate globals similar to a constructor
  (func $initialize
    (memory.init $colors (global.get $colorsOffset) (i32.const 0) (global.get $colorsLength))
  )
  ;; Return buffer of comma separated colors
  ;; black,brown,red,orange,yellow,green,blue,violet,grey,white
  (func (export "colors") (result i32 i32)
    (return (global.get $colorsOffset) (global.get $colorsLength))
  )
  (func $memcmp (param $s1 i32) (param $s2 i32) (param $n i32) (result i32) 
    (local $s1Cursor i32)
    (local $s2Cursor i32)
    (local.set $s1Cursor (local.get $s1))
    (local.set $s2Cursor (local.get $s2))
    (call $log_mem_as_utf8 (local.get $s1) (local.get $n))
    (call $log_mem_as_utf8 (local.get $s2) (local.get $n))
    (if (i32.gt_s (local.get $n) (i32.const 0)) (then
      (loop
        (if (i32.ne (i32.load8_u (local.get $s1Cursor)) (i32.load8_u (local.get $s2Cursor))) (then
          (if (i32.lt_u (i32.load8_u (local.get $s1Cursor)) (i32.load8_u (local.get $s2Cursor)) ) (then
            (return (i32.const -1))
          ) (else 
            (return (i32.const 1))
          ))
        ))
        (local.set $s1Cursor (i32.add (local.get $s1Cursor) (i32.const 1)))
        (local.set $s2Cursor (i32.add (local.get $s2Cursor) (i32.const 1)))
        (br_if 0 (i32.gt_s (local.tee $n (i32.sub (local.get $n (i32.const 1)))) (i32.const 0)))
      )  
    ))
    (i32.const 0)
  )
  ;; Given a valid resistor color, returns the associated value 
  (func (export "colorCode") (param $offset i32) (param $len i32) (result i32)
    (if (i32.and 
      (i32.eq (local.get $len) (global.get $blackLength)) 
      (i32.eqz (call $memcmp (local.get $offset) (i32.add (global.get $colorsOffset) (global.get $blackOffset)) (local.get $len)))
    ) (then 
      (return (global.get $blackValue))
    ) (else (if (i32.and 
      (i32.eq (local.get $len) (global.get $brownLength)) 
      (i32.eqz (call $memcmp (local.get $offset) (i32.add (global.get $colorsOffset) (global.get $brownOffset)) (local.get $len)))
    ) (then
      (return (global.get $brownValue))
    ) (else (if (i32.and 
      (i32.eq (local.get $len) (global.get $redLength)) 
      (i32.eqz (call $memcmp (local.get $offset) (i32.add (global.get $colorsOffset) (global.get $redOffset)) (local.get $len)))
    ) (then
      (return (global.get $redValue))
    ) (else (if (i32.and 
      (i32.eq (local.get $len) (global.get $orangeLength)) 
      (i32.eqz (call $memcmp (local.get $offset) (i32.add (global.get $colorsOffset) (global.get $orangeOffset)) (local.get $len)))
    ) (then
      (return (global.get $orangeValue))
    ) (else (if (i32.and 
      (i32.eq (local.get $len) (global.get $yellowLength)) 
      (i32.eqz (call $memcmp (local.get $offset) (i32.add (global.get $colorsOffset) (global.get $yellowOffset)) (local.get $len)))) 
    (then
      (return (global.get $yellowValue))
    ) (else (if (i32.and 
      (i32.eq (local.get $len) (global.get $greenLength)) 
      (i32.eqz (call $memcmp (local.get $offset) (i32.add (global.get $colorsOffset) (global.get $greenOffset)) (local.get $len)))) 
    (then
      (return (global.get $greenValue))
    ) (else (if (i32.and 
      (i32.eq (local.get $len) (global.get $blueLength)) 
      (i32.eqz (call $memcmp (local.get $offset) (i32.add (global.get $colorsOffset) (global.get $blueOffset)) (local.get $len)))) 
    (then
      (return (global.get $blueValue))
    ) (else (if (i32.and 
      (i32.eq (local.get $len) (global.get $violetLength)) 
      (i32.eqz (call $memcmp (local.get $offset) (i32.add (global.get $colorsOffset) (global.get $violetOffset)) (local.get $len)))) 
    (then
      (return (global.get $violetValue))
    ) (else (if (i32.and 
      (i32.eq (local.get $len) (global.get $greyLength)) 
      (i32.eqz (call $memcmp (local.get $offset) (i32.add (global.get $colorsOffset) (global.get $greyOffset)) (local.get $len)))) 
    (then
      (return (global.get $greyValue))
    ) (else (if (i32.and 
      (i32.eq (local.get $len) (global.get $whiteLength)) 
      (i32.eqz (call $memcmp (local.get $offset) (i32.add (global.get $colorsOffset) (global.get $whiteOffset)) (local.get $len)))) 
    (then
      (return (global.get $whiteValue))
    ))))))))))))))))))))
    (i32.const -1)
  )
)
