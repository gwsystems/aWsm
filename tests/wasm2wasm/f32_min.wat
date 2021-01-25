(module
  (type (;0;) (func (param f32 i32) (result f32)))
  (func $main (type 0) (param f32 i32) (result f32)
    f32.const 10
    local.get 0
    f32.min
	)
  (export "main" (func $main))
)