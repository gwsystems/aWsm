(loop $label$121
    (block $label$122
        (block $label$123
            ;; Break if ($13 & 1) == 0
            (br_if $label$123 (i32.eqz (i32.and (local.get $13) (i32.const 1))))
            (block $label$124
                (br_if $label$124 (local.get $16))
                (local.set $14 (local.get $20))
                (local.set $16 (i32.const 1))
                (br $label$122)
            )
            (local.set $8 (i32.eqz (local.get $11)))
            (br $label$118)
        )
        ;; $20++
        (local.set $20 (i64.add (local.get $20) (i64.const 1)))
        (block $label$125
            ;; Break if $18 > 124
            (br_if $label$125 (i32.gt_s (local.get $18) (i32.const 124)))
            ;; $13 = ($1 == 48)
            (local.set $13 (i32.eq (local.get $1) (i32.const 48)))
            (local.set $26 (i32.wrap_i64 (local.get $20)))
            ;; $11 == $3 + ($18 << 2). Multiply by 100?
            (local.set $11 (i32.add (local.get $3) (i32.shl (local.get $18) (i32.const 2))))
            ;; if (j) x[k] = x[k]*10 + c-'0';
            (block $label$126
                (br_if $label$126 (i32.eqz (local.get $19)))
                ;; $12 = $11 * 10 + $1 - '0';
                (local.set $12 (i32.add (i32.add (local.get $1) (i32.mul (i32.load (local.get $11)) (i32.const 10))) (i32.const -48)))
            )


            (local.set $25 (select (local.get $25) (local.get $26) (local.get $13)))
            (i32.store (local.get $11) (local.get $12))
            (local.set $11 (i32.const 1))
            (local.set $19
                (select
                    (i32.const 0)
                    (local.tee $1
                        (i32.add (local.get $19) (i32.const 1))
                    )
                    (local.tee $1
                        (i32.eq (local.get $1) (i32.const 9))
                    )
                )
            )
            ;; $18 += $1
            (local.set $18 (i32.add (local.get $18) (local.get $1)))
            (br $label$122)
        )
        ;; Break if $1 == '0'
        (br_if $label$122 (i32.eq (local.get $1) (i32.const 48)))
        ;; mem[$3 + 496] |= 1 
        (i32.store offset=496
            (local.get $3)
            (i32.or
                (i32.load offset=496 (local.get $3))
                (i32.const 1)
            )
        )
        (local.set $25 (i32.const 1116))
    )
    (block $label$127
        (block $label$128
            ;; Break if ($1 = mem[$9] == mem[$8])
            (br_if $label$128
                (i32.eq
                    (local.tee $1 (i32.load (local.get $9)))
                    (i32.load (local.get $8))
                )
            )
            ;; $9 = $1 + 1
            (i32.store (local.get $9) (i32.add (local.get $1) (i32.const 1)))
            (local.set $1 (i32.load8_u (local.get $1)))
            (br $label$127)
        )
        (local.set $1 (call $__shgetc (local.get $0)))
    )
    ;; $12 = $1 - '0'
    (local.set $12 (i32.add (local.get $1) (i32.const -48)))
    ;; Break if ($13 = $1 =='.')
    (br_if $label$121 (local.tee $13 (i32.eq (local.get $1) (i32.const 46))))
    ;; Break if $12 < 10U
    (br_if $label$121 (i32.lt_u (local.get $12) (i32.const 10)))
)




           (func $__shgetc (; 14 ;) (param $0 i32) (result i32)
           (local $1 i32)
           (local $2 i32)
           (local $3 i64)
           (local $4 i64)
           (local $5 i32)
           (local.set $3
            (i64.add
             (i64.load offset=96
              (local.get $0)
             )
             (i64.extend_i32_s
              (i32.sub
               (local.tee $1
                (i32.load offset=4
                 (local.get $0)
                )
               )
               (local.tee $2
                (i32.load offset=40
                 (local.get $0)
                )
               )
              )
             )
            )
           )
           (block $label$1
            (block $label$2
             (block $label$3
              (br_if $label$3
               (i64.eqz
                (local.tee $4
                 (i64.load offset=88
                  (local.get $0)
                 )
                )
               )
              )
              (br_if $label$2
               (i64.ge_s
                (local.get $3)
                (local.get $4)
               )
              )
             )
             (br_if $label$1
              (i32.gt_s
               (local.tee $2
                (call $__uflow
                 (local.get $0)
                )
               )
               (i32.const -1)
              )
             )
             (local.set $1
              (i32.load offset=4
               (local.get $0)
              )
             )
             (local.set $2
              (i32.load offset=40
               (local.get $0)
              )
             )
            )
            (i64.store offset=88
             (local.get $0)
             (i64.const -1)
            )
            (i32.store offset=84
             (local.get $0)
             (local.get $1)
            )
            (i64.store offset=96
             (local.get $0)
             (i64.add
              (local.get $3)
              (i64.extend_i32_s
               (i32.sub
                (local.get $2)
                (local.get $1)
               )
              )
             )
            )
            (return
             (i32.const -1)
            )
           )
           (local.set $3
            (i64.add
             (local.get $3)
             (i64.const 1)
            )
           )
           (local.set $5
            (i32.load offset=8
             (local.get $0)
            )
           )
           (block $label$4
            (block $label$5
             (block $label$6
              (br_if $label$6
               (i64.ne
                (local.tee $4
                 (i64.load offset=88
                  (local.get $0)
                 )
                )
                (i64.const 0)
               )
              )
              (local.set $1
               (i32.load offset=4
                (local.get $0)
               )
              )
              (br $label$5)
             )
             (br_if $label$5
              (i64.ge_s
               (local.tee $4
                (i64.sub
                 (local.get $4)
                 (local.get $3)
                )
               )
               (i64.extend_i32_s
                (i32.sub
                 (local.get $5)
                 (local.tee $1
                  (i32.load offset=4
                   (local.get $0)
                  )
                 )
                )
               )
              )
             )
             (i32.store offset=84
              (local.get $0)
              (i32.add
               (local.get $1)
               (i32.wrap_i64
                (local.get $4)
               )
              )
             )
             (br $label$4)
            )
            (i32.store offset=84
             (local.get $0)
             (local.get $5)
            )
           )
           (i64.store offset=96
            (local.get $0)
            (i64.add
             (local.get $3)
             (i64.extend_i32_s
              (i32.sub
               (local.tee $5
                (i32.load offset=40
                 (local.get $0)
                )
               )
               (local.get $1)
              )
             )
            )
           )
           (block $label$7
            (br_if $label$7
             (i32.lt_u
              (local.get $5)
              (local.get $1)
             )
            )
            (i32.store8
             (i32.add
              (local.get $1)
              (i32.const -1)
             )
             (local.get $2)
            )
           )
           (local.get $2)
          )

          (func $__toread (; 11 ;) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (i32.store offset=60
   (local.get $0)
   (i32.or
    (i32.add
     (local.tee $1
      (i32.load offset=60
       (local.get $0)
      )
     )
     (i32.const -1)
    )
    (local.get $1)
   )
  )
  (block $label$1
   (br_if $label$1
    (i32.eq
     (i32.load offset=20
      (local.get $0)
     )
     (i32.load offset=24
      (local.get $0)
     )
    )
   )
   (drop
    (call_indirect (type $i32_i32_i32_=>_i32)
     (local.get $0)
     (i32.const 0)
     (i32.const 0)
     (i32.load offset=32
      (local.get $0)
     )
    )
   )
  )
  (i32.store offset=24
   (local.get $0)
   (i32.const 0)
  )
  (i64.store offset=16
   (local.get $0)
   (i64.const 0)
  )
  (block $label$2
   (br_if $label$2
    (i32.eqz
     (i32.and
      (local.tee $1
       (i32.load
        (local.get $0)
       )
      )
      (i32.const 4)
     )
    )
   )
   (i32.store
    (local.get $0)
    (i32.or
     (local.get $1)
     (i32.const 32)
    )
   )
   (return
    (i32.const -1)
   )
  )
  (i32.store offset=8
   (local.get $0)
   (local.tee $2
    (i32.add
     (i32.load offset=40
      (local.get $0)
     )
     (i32.load offset=44
      (local.get $0)
     )
    )
   )
  )
  (i32.store offset=4
   (local.get $0)
   (local.get $2)
  )
  (i32.shr_s
   (i32.shl
    (local.get $1)
    (i32.const 27)
   )
   (i32.const 31)
  )
 )
 (func $__uflow (; 12 ;) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (global.set $global$0
   (local.tee $1
    (i32.sub
     (global.get $global$0)
     (i32.const 16)
    )
   )
  )
  (local.set $2
   (i32.const -1)
  )
  (block $label$1
   (br_if $label$1
    (call $__toread
     (local.get $0)
    )
   )
   (br_if $label$1
    (i32.ne
     (call_indirect (type $i32_i32_i32_=>_i32)
      (local.get $0)
      (i32.add
       (local.get $1)
       (i32.const 15)
      )
      (i32.const 1)
      (i32.load offset=28
       (local.get $0)
      )
     )
     (i32.const 1)
    )
   )
   (local.set $2
    (i32.load8_u offset=15
     (local.get $1)
    )
   )
  )
  (global.set $global$0
   (i32.add
    (local.get $1)
    (i32.const 16)
   )
  )
  (local.get $2)
 )
