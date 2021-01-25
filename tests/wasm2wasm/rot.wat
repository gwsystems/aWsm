(module
  (type (;0;) (func (param i32 i32) (result i32)))
  (func $main (type 0) (param i32 i32) (result i32)
    local.get 0
	  local.get 1
    i32.rotl
	)
  (export "main" (func $main))
)