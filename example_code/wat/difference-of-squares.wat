(module
  ;; The name prefixed with $ is used to internally refer to functions via the call instruction
  ;; The string in the export instruction is the name of the export made available to the
  ;; embedding environment (in this case, Node.js). This is used by our test runner Jest.
  (func $squareOfSum (export "squareOfSum") (param $max i32) (result i32)
    (local $sum i32)
    (local $i i32)
    (local.set $sum (i32.const 0))
    (local.set $i (i32.const 1))
    (if (i32.le_s (local.get $i) (local.get $max )) (then
      (loop
        (local.set $sum (i32.add (local.get $sum) (local.get $i)))
        (br_if 0 (i32.le_s (local.tee $i (i32.add (local.get $i) (i32.const 1))) (local.get $max )))
      )
    ))
    (i32.mul (local.get $sum) (local.get $sum))
  )
  (func $sumOfSquares (export "sumOfSquares") (param $max i32) (result i32)
    (local $sum i32)
    (local $i i32)
    (local.set $sum (i32.const 0))
    (local.set $i (i32.const 1))
    
    (if (i32.le_s (local.get $i) (local.get $max )) (then
      (loop
        (local.set $sum (i32.add (local.get $sum) (i32.mul (local.get $i) (local.get $i))))
        (br_if 0 (i32.le_s (local.tee $i (i32.add (local.get $i) (i32.const 1))) (local.get $max )))
      )
    ))
    (local.get $sum)
  )
  (func (export "difference") (param $max i32) (result i32)
    (i32.sub
      (call $squareOfSum (local.get $max))
      (call $sumOfSquares (local.get $max))
    )
  )
)
