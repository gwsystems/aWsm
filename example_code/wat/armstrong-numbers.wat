(module
  ;; Uses ints instead of floats
  ;; Assumes x and y are > 0
  (func $ipow (param $x i32) (param $y i32) (result i32)
    (local $sum i32)
    (local.set $sum (i32.const 1))
    (if (i32.gt_u (local.get $y) (i32.const 0)) (then
      (loop
        (local.set $sum (i32.mul (local.get $sum) (local.get $x)))
        (br_if 0 (i32.gt_u (local.tee $y (i32.sub (local.get $y) (i32.const 1))) (i32.const 0)))  
      )  
    ))
    (local.get $sum)
  )
  
  ;; returns 1 if armstrong number, 0 otherwise
  (func (export "isArmstrongNumber") (param $candidate i32) (result i32)
    (local $temp i32)
    (local $sum i32)
    (local $digitCount i32)
    
    ;; Count Digits
    (local.set $digitCount (i32.const 0))
    (local.set $temp (local.get $candidate))
    (if (i32.gt_u (local.get $temp) (i32.const 0)) (then
      (loop
        (local.set $digitCount (i32.add (local.get $digitCount) (i32.const 1)))
        (br_if 0 (i32.gt_u (local.tee $temp (i32.div_u (local.get $temp) (i32.const 10))) (i32.const 0)))  
      )
    ))
    ;; Calculate armstrong sum
    (local.set $sum (i32.const 0))
    (local.set $temp (local.get $candidate))
    (if (i32.gt_u (local.get $temp) (i32.const 0)) (then
      (loop
        (local.set $sum (i32.add (local.get $sum) (call $ipow 
          (i32.rem_u (local.get $temp) (i32.const 10)) 
          (local.get $digitCount)
        )))
        (br_if 0 (i32.gt_u (local.tee $temp (i32.div_u (local.get $temp) (i32.const 10))) (i32.const 0)))
      )  
    ))
    (i32.eq (local.get $candidate) (local.get $sum))
  )
)
