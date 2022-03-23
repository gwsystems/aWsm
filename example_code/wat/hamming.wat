(module
  (memory (export "mem") 1)
  (func (export "compute") 
    (param $firstOffset i32) (param $firstLength i32) (param $secondOffset i32) (param $secondLength i32) (result i32)
    (local $i i32)
    (local $distance i32)
    (if (i32.ne (local.get $firstLength) (local.get $secondLength)) (then 
      (return (i32.const -1))
    ))
    (local.set $i (i32.const 0))
    (local.set $distance (i32.const 0))
    (loop
      (if (i32.ne
        (i32.load8_u (i32.add (local.get $firstOffset) (local.get $i)))
        (i32.load8_u (i32.add (local.get $secondOffset) (local.get $i)))
      ) (then
        (local.set $distance (i32.add (local.get $distance) (i32.const 1)))
      ))
      (br_if 0 (i32.lt_u (local.tee $i (i32.add (local.get $i) (i32.const 1))) (local.get $firstLength)))
    )
    (local.get $distance)
  )
)
