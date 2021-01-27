(module
  (type (;0;) (func (param T) (result i32)))
  (func $main (type 0) (param T) (result i32)
    local.get 0
	  T.eqz
	)
  (export "main" (func $main))
)