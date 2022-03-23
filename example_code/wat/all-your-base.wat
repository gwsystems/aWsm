(module
  (memory (export "mem") 1)
  
  ;; Status codes returned as res[2]
  (global $ok i32 (i32.const 0))
  (global $inputHasWrongFormat i32 (i32.const -1))
  (global $wrongInputBase i32 (i32.const -2))
  (global $wrongOutputBase i32 (i32.const -3))
  ;; Returns offset and length of resulting u32[] and a return status code
  (func (export "convert") (param $arrOffset i32) (param $arrLength i32) (param $inputBase i32) (param $outputBase i32) (result i32 i32 i32)
    (local $sum i32)
    (local $buffer i32)
    (local $resultLength i32)
    (local $digit i32)
    (local $i i32)
    (if (i32.lt_s (local.get $inputBase) (i32.const 2)) (then
      (return (i32.const 0) (i32.const 0) (global.get $wrongInputBase))
    ))
    (if (i32.lt_s (local.get $outputBase) (i32.const 2)) (then
      (return (i32.const 0) (i32.const 0) (global.get $wrongOutputBase))
    ))
    (if (i32.lt_s (local.get $arrLength) (i32.const 1)) (then 
      ;; Empty or negative array length
      (return (i32.const 0) (i32.const 0) (global.get $inputHasWrongFormat))
    ) (else (if (i32.and (i32.eq (local.get $arrLength) (i32.const 1)) (i32.eqz (i32.load (local.get $arrOffset)))) (then
      ;; input is [0], so valid
      (return (local.get $arrOffset) (local.get $arrLength) (global.get $ok))
    ) (else (if (i32.eqz (i32.load (local.get $arrOffset))) (then
      ;; Leading 0
      (return (i32.const 0) (i32.const 0) (global.get $inputHasWrongFormat))
    ))))))
    
    ;; Accumulate array into a decimal representation
    (local.set $i (i32.const 0))
    (local.set $sum (i32.const 0))
    (loop
      (local.set $digit (i32.load (i32.add (local.get $arrOffset) (i32.mul (local.get $i) (i32.const 4)))))
      ;; if (digit < 0) return -1
      ;; if (digit >= inputBase) return -1
      (if (i32.or 
        (i32.lt_s (local.get $digit) (i32.const 0))
        (i32.ge_s (local.get $digit) (local.get $inputBase))
      ) (then
        (return (i32.const 0) (i32.const 0) (global.get $inputHasWrongFormat))
      ))
      ;; sum * inputBase + digit
      (local.set $sum (i32.add (i32.mul (local.get $sum) (local.get $inputBase)) (local.get $digit)))
    
      (br_if 0 (i32.lt_s (local.tee $i (i32.add (local.get $i) (i32.const 1))) (local.get $arrLength)))
    )
    ;; Assumption: A sum of zero should not be possible, as we checked that the leading digit was non-zero
    ;; Figure out digits count in target radix
    ;; while ((sum /= outputBase) > 0) resultLength++
    (local.set $buffer (local.get $sum))
    (local.set $resultLength (i32.const 0))
    (loop
      (local.set $resultLength (i32.add (local.get $resultLength) (i32.const 1)))
      (br_if 0 (i32.gt_s (local.tee $buffer (i32.div_s (local.get $buffer) (local.get $outputBase))) (i32.const 0)))
    )
    ;; Start at (resultLength - 1) and go right to left building up result
    (local.set $buffer (local.get $sum))
    (local.set $i (i32.sub (local.get $resultLength) (i32.const 1)))
    
    (loop
      (i32.store 
        (i32.add (local.get $arrOffset) (i32.mul (local.get $i) (i32.const 4))) 
        (i32.rem_s (local.get $buffer) (local.get $outputBase))
      )
      (local.set $i (i32.sub (local.get $i) (i32.const 1)))
      
      (br_if 0 (i32.gt_s (local.tee $buffer (i32.div_s (local.get $buffer) (local.get $outputBase))) (i32.const 0)))
    )
    ;; Return (arrOffset, resultLength, 0)
    (return (local.get $arrOffset) (local.get $resultLength) (global.get $ok))
  )
)
