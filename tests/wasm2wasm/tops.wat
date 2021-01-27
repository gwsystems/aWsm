(module
  (type (;0;) (func (param i64) (result i32)))
  (func $main (type 0) (param i64) (result i32)
    local.get 0
	  i64.eqz
	)
  (export "main" (func $main))
)