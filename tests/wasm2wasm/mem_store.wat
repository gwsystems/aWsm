(module
  (type (;0;) (func (param i32 f64) ))
  (func $main (type 0) (param i32 f64)
    local.get 0
    local.get 1
	  f64.store
	)
  (table (;0;) 1 1 funcref)
  (memory (;0;) 2)
  (export "main" (func $main))
)