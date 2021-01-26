(module
  (type (;0;) (func (param f64 f64) (result i32)))
  (func $main (type 0) (param f64 f64) (result i32)
    local.get 0
	  local.get 1
	f64.ge
	)
  (export "main" (func $main))
)