(module
  (type (;0;) (func (param f64) (result f64)))
  (func $main (type 0) (param f64) (result f64)
    local.get 0
	  f64.trunc
	)
  (export "main" (func $main))
)