(module
  (func $isDegenate (param f32 f32 f32) (result i32)
    (i32.or
      (i32.or
        (f32.eq (local.get 0) (f32.const 0.0))
        (i32.or
          (f32.eq (local.get 1) (f32.const 0.0))
          (f32.eq (local.get 2) (f32.const 0.0))
        )
      )
      (i32.or
        (i32.or
          (f32.le
            (f32.add (local.get 0) (local.get 1))
            (local.get 2)
          )
          (f32.le
            (f32.add (local.get 0) (local.get 2))
            (local.get 1)
          )
        )
        (f32.le
          (f32.add (local.get 1) (local.get 2))
          (local.get 0)
        )
      )
    )
  )
  (func (export "isEquilateral") (param f32 f32 f32) (result i32)
    (if (call $isDegenate (local.get 0) (local.get 1) (local.get 2)) (then (return (i32.const 0))))
    (i32.and
      (f32.eq (local.get 0) (local.get 1))
      (f32.eq (local.get 1) (local.get 2))
    )
  )
  (func $isIsosceles (export "isIsosceles") (param f32 f32 f32) (result i32)
    (if (call $isDegenate (local.get 0) (local.get 1) (local.get 2)) (then (return (i32.const 0))))
    (i32.or
      (f32.eq (local.get 0) (local.get 1))
      (i32.or
        (f32.eq (local.get 0) (local.get 2))
        (f32.eq (local.get 1) (local.get 2))
      )
    )
  )
  (func (export "isScalene") (param f32 f32 f32) (result i32)
    (if (call $isDegenate (local.get 0) (local.get 1) (local.get 2)) (then (return (i32.const 0))))
    
    (i32.and
      (f32.ne (local.get 0) (local.get 1))
      (i32.and
        (f32.ne (local.get 0) (local.get 2))
        (f32.ne (local.get 1) (local.get 2))
      )
    )
  )
)
