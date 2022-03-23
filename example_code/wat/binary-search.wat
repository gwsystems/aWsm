(module
  (memory (export "mem") 1)
 
  ;; Assumes size of i32
  (func (export "find") (param $base i32) (param $nelems i32) (param $needle i32) (result i32)
    (local $low i32)
    (local $high i32)
    (local $mid i32)
    (local $midVal i32)
    (local.set $low (i32.const 0))
    (local.set $high (i32.sub (local.get $nelems) (i32.const 1)))
    (if (i32.le_s (local.get $low) (local.get $high)) (then (loop
        (local.set $mid (i32.div_s (i32.add (local.get $high) (local.get $low)) (i32.const 2)))
        (local.set $midVal (i32.load (i32.add (local.get $base) (i32.mul (local.get $mid) (i32.const 4)))))
        (if (i32.eq (local.get $needle) (local.get $midVal))( then
          (return (local.get $mid))
        ) (else (if (i32.lt_s (local.get $needle) (local.get $midVal)) (then
          (local.set $high (i32.sub (local.get $mid) (i32.const 1)))
        ) (else (if (i32.gt_s (local.get $needle) (local.get $midVal)) (then
          (local.set $low (i32.add (local.get $mid) (i32.const 1)))
        ))))))
        (br_if 0 (i32.le_s (local.get $low) (local.get $high)))
    )))
    (i32.const -1)
  )
)
