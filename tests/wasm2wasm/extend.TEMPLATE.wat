(module
  (type (;0;) (func (param T T) (result R)))
  (func $main (type 0) (param i64 T) (result i64)
    local.get 0
	  i64.extend32_s
	)
  (table (;0;) 1 1 funcref)
  (memory (;0;) 2)
  (export "main" (func $main))
)