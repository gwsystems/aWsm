(module
  (type (;0;) (func (param f64 i32) (result f64)))
  (func $main (type 0) (param f64 i32) (result f64)
    f64.const 10
    local.get 0
    f64.max
	)
  (export "main" (func $main))
)