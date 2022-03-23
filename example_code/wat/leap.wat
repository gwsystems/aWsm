(module
  ;; Returns 1 if leap year, 0 otherwise
  (func (export "isLeap") (param $year i32) (result i32)
    (i32.and
      (i32.eqz (i32.rem_u (local.get $year) (i32.const 4))) 
      (i32.or
        (i32.ne (i32.rem_u (local.get $year) (i32.const 100)) (i32.const 0)) 
        (i32.eqz (i32.rem_u (local.get $year) (i32.const 400)))))
  )  
)
