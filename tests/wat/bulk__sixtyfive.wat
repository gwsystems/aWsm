;; Test that the data segment index is properly encoded as an unsigned (not
;; signed) LEB.
(module
	;; 65 data segments. 64 is the smallest positive number that is encoded
	;; differently as a signed LEB.
	(data "") (data "") (data "") (data "") (data "") (data "") (data "") (data "")
	(data "") (data "") (data "") (data "") (data "") (data "") (data "") (data "")
	(data "") (data "") (data "") (data "") (data "") (data "") (data "") (data "")
	(data "") (data "") (data "") (data "") (data "") (data "") (data "") (data "")
	(data "") (data "") (data "") (data "") (data "") (data "") (data "") (data "")
	(data "") (data "") (data "") (data "") (data "") (data "") (data "") (data "")
	(data "") (data "") (data "") (data "") (data "") (data "") (data "") (data "")
	(data "") (data "") (data "") (data "") (data "") (data "") (data "") (data "")
	(data "")
	(func (data.drop 64))
)
