	.text
	.file	"main.c"
	.section	.text.f,"",@
	.hidden	f                       # -- Begin function f
	.globl	f
	.type	f,@function
f:                                      # @f
	.functype	f (i32, i32) -> (i32)
	.local  	i32
# %bb.0:                                # %entry
	global.get	__stack_pointer
	i32.const	16
	i32.sub 
	local.tee	2
	local.get	0
	i32.store	12
	local.get	2
	local.get	1
	i32.store	8
	local.get	2
	i32.load	12
	local.get	2
	i32.load	8
	i32.const	31
	i32.and 
	i32.shl 
	local.get	2
	i32.load	12
	i32.const	0
	local.get	2
	i32.load	8
	i32.sub 
	i32.const	31
	i32.and 
	i32.shr_s
	i32.or  
                                        # fallthrough-return
	end_function
.Lfunc_end0:
	.size	f, .Lfunc_end0-f
                                        # -- End function
	.section	.text.g,"",@
	.hidden	g                       # -- Begin function g
	.globl	g
	.type	g,@function
g:                                      # @g
	.functype	g (i32, i32) -> (i32)
	.local  	i32
# %bb.0:                                # %entry
	global.get	__stack_pointer
	i32.const	16
	i32.sub 
	local.tee	2
	local.get	0
	i32.store	12
	local.get	2
	local.get	1
	i32.store	8
	local.get	2
	i32.load	12
	local.get	2
	i32.load	8
	i32.const	31
	i32.and 
	i32.shr_s
	local.get	2
	i32.load	12
	i32.const	0
	local.get	2
	i32.load	8
	i32.sub 
	i32.const	31
	i32.and 
	i32.shl 
	i32.or  
                                        # fallthrough-return
	end_function
.Lfunc_end1:
	.size	g, .Lfunc_end1-g
                                        # -- End function
	.section	.text.h,"",@
	.hidden	h                       # -- Begin function h
	.globl	h
	.type	h,@function
h:                                      # @h
	.functype	h (f32) -> (f32)
	.local  	i32
# %bb.0:                                # %entry
	global.get	__stack_pointer
	i32.const	16
	i32.sub 
	local.tee	1
	local.get	0
	f32.store	12
	local.get	1
	f32.load	12
	f64.promote_f32
	f64.floor
	f32.demote_f64
                                        # fallthrough-return
	end_function
.Lfunc_end2:
	.size	h, .Lfunc_end2-h
                                        # -- End function
	.section	.text.i,"",@
	.hidden	i                       # -- Begin function i
	.globl	i
	.type	i,@function
i:                                      # @i
	.functype	i (i32, i32) -> (i32)
	.local  	i32
# %bb.0:                                # %entry
	global.get	__stack_pointer
	i32.const	16
	i32.sub 
	local.tee	2
	local.get	0
	i32.store	12
	local.get	2
	local.get	1
	i32.store	8
	i32.const	linear_mem
	local.get	2
	i32.load	12
	i32.const	2
	i32.shl 
	i32.add 
	local.get	2
	i32.load	8
	i32.store	0
	i32.const	0
                                        # fallthrough-return
	end_function
.Lfunc_end3:
	.size	i, .Lfunc_end3-i
                                        # -- End function
	.hidden	linear_mem              # @linear_mem
	.type	linear_mem,@object
	.section	.bss.linear_mem,"",@
	.globl	linear_mem
	.p2align	4
linear_mem:
	.skip	400
	.size	linear_mem, 400

	.ident	"clang version 12.0.0 (https://github.com/Jacarte/llvm-project.git b4f6be84dff4c817ca7de248d0fd1ce16504a2c0)"
	.globaltype	__stack_pointer, i32
	.section	.custom_section.producers,"",@
	.int8	1
	.int8	12
	.ascii	"processed-by"
	.int8	1
	.int8	5
	.ascii	"clang"
	.int8	93
	.ascii	"12.0.0 (https://github.com/Jacarte/llvm-project.git b4f6be84dff4c817ca7de248d0fd1ce16504a2c0)"
	.section	.bss.linear_mem,"",@
