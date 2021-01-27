(module
  (type (;0;) (func (param i32 i32) (result i32)))
  (type (;1;) (func))
  (type (;2;) (func (result i32)))
  (import "env" "printf" (func $printf (type 0)))
  (func $__original_main (type 2) (result i32)
    (local i32 i32 i32 i32)
    global.get 0
    i32.const 16
    i32.sub
    local.tee 0
    global.set 0
    i32.const -1
    local.set 1
    block  ;; label = @1
      loop  ;; label = @2
        local.get 1
        i32.const 1
        i32.add
        local.tee 1
        local.get 1
        i32.mul
        local.tee 2
        i32.const 1000000
        i32.rem_u
        local.set 3
        local.get 2
        i32.const 2147483647
        i32.eq
        br_if 1 (;@1;)
        local.get 3
        i32.const 269696
        i32.ne
        br_if 0 (;@2;)
      end
    end
    local.get 0
    local.get 1
    i32.store
    i32.const 1024
    local.get 0
    call $printf
    drop
    local.get 0
    i32.const 16
    i32.add
    global.set 0
    i32.const 0)
  (func $main (type 0) (param i32 i32) (result i32)
    call $__original_main)
  (table (;0;) 1 1 funcref)
  (memory (;0;) 2)
  (global (;0;) (mut i32) (i32.const 66624))
  (global (;1;) i32 (i32.const 1079))
  (global (;2;) i32 (i32.const 1024))
  (global (;3;) i32 (i32.const 66624))
  (global (;4;) i32 (i32.const 1024))
  (export "memory" (memory 0))
  (export "__data_end" (global 1))
  (export "__global_base" (global 2))
  (export "__heap_base" (global 3))
  (export "__dso_handle" (global 4))
  (export "main" (func $main))
  (export "__original_main" (func $__original_main))
  (data (;0;) (i32.const 1024) "The smallest number whose square ends in 269696 is %d\0a\00"))
