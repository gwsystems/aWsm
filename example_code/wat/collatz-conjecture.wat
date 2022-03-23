(module
  ;; (import "console" "log_i32_s" (func $log_i32_s (param i32)))
  (func (export "steps") (param $number i32) (result i32)
    (local $steps i32)
    (local.set $steps (i32.const 0))
    ;; Return -1 to indicate error if inputs is <= 0
    (if (i32.le_s (local.get $number) (i32.const 0)) (then
      (return (i32.const -1))
    ))
    (if (i32.gt_s (local.get $number) (i32.const 1)) (then
      (loop
        ;; (call $log_i32_s (local.get $number))
        
        (if (i32.eqz (i32.rem_s (local.get $number) (i32.const 2))) (then
          (local.set $number (i32.div_s (local.get $number) (i32.const 2)))
        ) (else 
          (local.set $number 
            (i32.add
              (i32.mul (local.get $number) (i32.const 3))
              (i32.const 1)
            )
          )
        ))
        (local.set $steps (i32.add (local.get $steps) (i32.const 1)))
        
        (br_if 0 (i32.gt_s (local.get $number) (i32.const 1)))
      )
    ))
    (local.get $steps)
  )
)
