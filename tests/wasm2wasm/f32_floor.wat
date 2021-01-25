(module
  (type (;0;) (func (param f32 i32) (result f32)))
  (func $main (type 0) (param f32 i32) (result f32)
    local.get 0
    f32.floor
	)
  (export "main" (func $main))
)