(module
  (memory 1)
  (global $head (mut i32) (i32.const -1))
  (global $tail (mut i32) (i32.const -1))
  (global $capacity (mut i32) (i32.const 0))
  (global $i32Size i32 (i32.const 4))
  ;; capacity: the number of elements to store
  ;; elementSize: the size of the element to store in bytes
  ;; Does not support resizing circular buffer. Wipes all data
  (func (export "init") (param $newCapacity i32) (result i32)
    ;; a WebAssembly page is 4096 bytes, so up to 1024 i32s
    (if (i32.gt_s (local.get $newCapacity) (i32.const 1024)) (then
      (return (i32.const -1))
    ))
    (global.set $head (i32.const -1))
    (global.set $tail (i32.const -1))
    (global.set $capacity (local.get $newCapacity))
    (i32.const 0)
  )
  (func (export "clear")
    (global.set $head (i32.const -1))
    (global.set $tail (i32.const -1))
  )
  (func (export "write") (param $elem i32) (result i32)
    (local $temp i32)
    ;; Table has capacity of zero
    (if (i32.eq (global.get $capacity) (i32.const 0)) (then
      (return (i32.const -1))
    ))
    ;; if head is -1, circular buffer is empty
    (if (i32.eq (global.get $head) (i32.const -1)) (then
      (global.set $head (i32.const 0))
      (global.set $tail (i32.const 0))
      (i32.store (i32.mul (global.get $tail) (global.get $i32Size)) (local.get $elem))
    ) (else 
      (local.set $temp (i32.rem_u (i32.add (global.get $tail) (i32.const 1)) (global.get $capacity)))
      ;; If tail is one less than head, we're full
      (if (i32.eq (local.get $temp) (global.get $head)) (then
        (return (i32.const -1))
      ))
      (global.set $tail (local.get $temp))
      (i32.store (i32.mul (global.get $tail) (global.get $i32Size)) (local.get $elem))
    ))
    (i32.const 0)
  )
  (func (export "forceWrite") (param $elem i32) (result i32)
    (local $temp i32)
    ;; Table has capacity of zero
    (if (i32.eq (global.get $capacity) (i32.const 0)) (then
      (return (i32.const -1))
    ))
    ;; if head is -1, circular buffer is empty
    (if (i32.eq (global.get $head) (i32.const -1)) (then
      (global.set $head (i32.const 0))
      (global.set $tail (i32.const 0))
      (i32.store (i32.mul (global.get $tail) (global.get $i32Size)) (local.get $elem))
    ) (else 
      (local.set $temp (i32.rem_u (i32.add (global.get $tail) (i32.const 1)) (global.get $capacity)))
      ;; If tail is one less than head, we're full. Advance head and overwrite
      (if (i32.eq (local.get $temp) (global.get $head)) (then
        (global.set $head (i32.rem_u (i32.add (global.get $head) (i32.const 1)) (global.get $capacity)))
      ))
      (global.set $tail (local.get $temp))
      (i32.store (i32.mul (global.get $tail) (global.get $i32Size)) (local.get $elem))
    ))
    (i32.const 0)
  )
  ;; Go-style error handling type (i32,i32)
  (func (export "read") (result i32 i32)
    (local $result i32)
    ;; if head is -1, circular buffer is empty
    (if (i32.eq (global.get $head) (i32.const -1)) (then
      (return (i32.const -1) (i32.const -1))
    ))
    ;; if head and tail are equal, we have one element
    (if (i32.eq (global.get $head) (global.get $tail)) (then
      (local.set $result (i32.load (i32.mul (global.get $head) (global.get $i32Size))))
      (global.set $head (i32.const -1))
      (global.set $tail (i32.const -1))
      (return (local.get $result) (i32.const 0) )
    ))
    (local.set $result (i32.load (i32.mul (global.get $head) (global.get $i32Size))))
    (global.set $head (i32.rem_u (i32.add (global.get $head) (i32.const 1)) (global.get $capacity)))
    (return (local.get $result) (i32.const 0))
  )
)
