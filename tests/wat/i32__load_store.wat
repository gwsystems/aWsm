(module
	(memory 1)

	(func $i32__load_store (export "i32__load_store") (param $addr i32) (param $newVal i32) (result i32)
		(local $oldVal i32)
		(local.set $oldVal (i32.load (local.get $addr)))
		(i32.store (local.get $addr) (local.get $newVal))
		(local.get $oldVal)
	)
)
