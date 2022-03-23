(module
  (import "console" "log_i32_s" (func $log_i32_s (param i32)))
  (memory (export "mem") 1)
  (data $pling "Pling")
  (data $plang "Plang")
  (data $plong "Plong")
  (global $plingLength i32 (i32.const 5))
  (global $plangLength i32 (i32.const 5))
  (global $plongLength i32 (i32.const 5))
  ;; Only handles decimal
  ;; Modified to return base address and length as a tuple!
  (func $itoa_s (param $x i32) (param $offset i32) (param $radix i32) (result i32 i32)
    (local $low i32)
    (local $high i32)
    (local $temp i32)
    (local $length i32)
    (local.set $low (local.get $offset))
    (if (i32.lt_s (local.get $x) (i32.const 0)) (then
      ;; Append negative sign
      (i32.store8 (local.get $low) (i32.const 45))
      (local.set $low (i32.add (local.get $low) (i32.const 1)))
      (local.set $x (i32.mul (local.get $x) (i32.const -1)))
    ))
  
    (local.set $high (local.get $low))
    (if (i32.gt_s (local.get $x) (i32.const 0)) (then
      (loop
        (i32.store8 (local.get $high) (i32.add (i32.const 48) (i32.rem_u (local.get $x) (local.get $radix))))
        (local.set $high (i32.add (local.get $high) (i32.const 1)))
        (br_if 0 (i32.gt_s (local.tee $x (i32.div_u (local.get $x) (local.get $radix))) (i32.const 0)))
      )
    ))
  
    (local.set $length (i32.sub (local.get $high) (local.get $offset)))
    
    (local.set $high (i32.sub (local.get $high) (i32.const 1)))
    (if (i32.gt_u (local.get $high) (local.get $low)) (then
      (loop
        (local.set $temp (i32.load8_u (local.get $low)))
        (i32.store8 (local.get $low) (i32.load8_u (local.get $high)))
        (i32.store8 (local.get $high) (local.get $temp))
        (br_if 0 (i32.gt_u 
          (local.tee $high (i32.sub (local.get $high) (i32.const 1))) 
          (local.tee $low (i32.add (local.get $low) (i32.const 1))) 
        ))
      )
    ))
    (local.get $offset) (local.get $length)
  )
  (func (export "convert") (param $input i32) (result i32 i32)
    (local $resultOffset i32)
    (local $resultLength i32)
    (local.set $resultOffset (i32.const 512))
    (local.set $resultLength (i32.const 0))
    
    (if (i32.eqz (i32.rem_s (local.get $input) (i32.const 3)))(then
      (memory.init $pling (i32.add (local.get $resultOffset) (local.get $resultLength)) (i32.const 0) (global.get $plingLength))
      (local.set $resultLength (i32.add (local.get $resultLength) (global.get $plingLength)))
    ))
    (if (i32.eqz (i32.rem_s (local.get $input) (i32.const 5)))(then
      (memory.init $plang (i32.add (local.get $resultOffset) (local.get $resultLength)) (i32.const 0) (global.get $plangLength))
      (local.set $resultLength (i32.add (local.get $resultLength) (global.get $plangLength)))
    ))
    (if (i32.eqz (i32.rem_s (local.get $input) (i32.const 7)))(then
      (memory.init $plong (i32.add (local.get $resultOffset) (local.get $resultLength)) (i32.const 0) (global.get $plongLength))
      (local.set $resultLength (i32.add (local.get $resultLength) (global.get $plongLength)))
    ))
    (if (i32.eqz (local.get $resultLength))(then
      (return (call $itoa_s (local.get $input) (local.get $resultOffset) (i32.const 10)))
    ))
    
    (return (local.get $resultOffset) (local.get $resultLength))
  )
)
