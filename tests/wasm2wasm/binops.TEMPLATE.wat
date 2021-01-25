(module
  (type (;0;) (func (param T T) (result R)))
  (func $main (type 0) (param T T) (result R)
    local.get 0
	local.get 1
	OP
	)
  (export "main" (func $main))
)