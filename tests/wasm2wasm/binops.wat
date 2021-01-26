(module
  (type (;0;) (func (param f64 f64) (result f64)))
  (func $main (type 0) (param f64 f64) (result f64)
    local.get 0
	local.get 1
	f64.copysign
	)
  (export "main" (func $main))
)