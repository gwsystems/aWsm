;; elem.drop
(module
	(import "wasi_snapshot_preview1" "proc_exit" (func $proc_exit (param i32)))
	(table 1 funcref)
	(func $f)

	;; error: unexpected token (, expected ).
	;; (elem $p funcref (ref.func $f))
	;; (elem $a (table 0) (i32.const 0) func $f)

	(func (export "drop_passive") (elem.drop $p))
	(func (export "init_passive") (param $len i32)
	(table.init $p (i32.const 0) (i32.const 0) (local.get $len))
	)

	(func (export "drop_active") (elem.drop $a))
	(func (export "init_active") (param $len i32)
	(table.init $a (i32.const 0) (i32.const 0) (local.get $len))
	)

	(func (export "_start")
		(call $init_passive (i32.const 1))
		(call $drop_passive)
		(call $drop_passive)
		(call $init_passive (i32.const 0))
		;; (assert_trap (invoke "init_passive" (i32.const 1)) "out of bounds table access")
		(call $init_passive (i32.const 0))
		(call $drop_active)
		(call $init_active (i32.const 0)))
		;; (assert_trap (invoke "init_active" (i32.const 1)) "out of bounds table access")
		(call $init_active (i32.const 0))
	)
)


;; ;; Test that the elem segment index is properly encoded as an unsigned (not
;; ;; signed) LEB.
;; (module
;;   ;; 65 elem segments. 64 is the smallest positive number that is encoded
;;   ;; differently as a signed LEB.
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref) (elem funcref) (elem funcref) (elem funcref)
;;   (elem funcref)
;;   (func (elem.drop 64)))

;; ;; No table is required for the elem.drop instruction.
;; (module (elem funcref (ref.func 0)) (func (elem.drop 0)))
