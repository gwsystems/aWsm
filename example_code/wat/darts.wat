(module
;;   (import "console" "log_f32" (func $log_f32 (param f32)))
  (global $outerCircleRadius f32 (f32.const 10.0))
  (global $middleCircleRadius f32 (f32.const 5.0))
  (global $innerCircleRadius f32 (f32.const 1.0))
  (global $outerCirclePoints i32 (i32.const 1))
  (global $middleCirclePoints i32 (i32.const 5))
  (global $innerCirclePoints i32 (i32.const 10))
  (func (export "score") (param $x f32) (param $y f32) (result i32)
    (local $distance f32)
    ;; (call $log_f32 (local.get $x))
    ;; (call $log_f32 (local.get $y))
    (local.set $distance 
      (f32.sqrt
        (f32.add
          (f32.mul (local.get $x) (local.get $x))
          (f32.mul (local.get $y) (local.get $y))
        )
      )
    )
    ;; (call $log_f32 (local.get $distance))
    (if (f32.le (local.get $distance) (global.get $innerCircleRadius)) (then
      (return (global.get $innerCirclePoints))
    ) (else (if (f32.le (local.get $distance) (global.get $middleCircleRadius)) (then
      (return (global.get $middleCirclePoints))
    ) (else (if (f32.le (local.get $distance) (global.get $outerCircleRadius)) (then
      (return (global.get $outerCirclePoints))
    ))))))
    (i32.const 0)
  )
)
