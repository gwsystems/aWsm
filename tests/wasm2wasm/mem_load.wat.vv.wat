(module
  (type (;0;) (func (param i32 f64) (result f64)))
  (func (;0;) (type 0) (param i32 f64) (result f64)
    local.get 0
    f64.load)
  (table (;0;) 1 1 funcref)
  (memory (;0;) 2)
  (export "main" (func 0)))
