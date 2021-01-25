(module
  (type (;0;) (func (param i32 i32) (result i32)))
  (func $main (type 0) (param i32 i32) (result i32)
    local.get 0
	i32.const 10
	i32.add
	)
  (export "main" (func $main))
)