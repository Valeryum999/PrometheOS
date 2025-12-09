# Declare a header as in the Multiboot Standard.
.section .multiboot2
.align 8

header_start:
	# Multiboot2 header
    .long 0xE85250D6                    # magic
    .long 0                             # architecture = i386
    .long header_end - header_start     # size
    .long -(0xE85250D6 + 0 + (header_end - header_start))

    # Framebuffer request info
    .align 8
    .long 1
    .long 12
    .long 8 

    # Framebuffer request for 1024x768 32bpp
    .align 8
    .long 5                             
    .long 20                            
    .long 1024
	.long 768
	.long 32                          

    # --- End tag ---
    .align 8
    .long 0                             # uint32 type = END
    .long 8                             # uint32 size = 8
header_end:

# The higher kernel entry point.
.section .lowentry
.global _start
.type _start, @function
_start:
	xor %ecx, %ecx
	add $0x7ff00, %ecx
	mov %ecx, %esp
	call _entry
	call _init
	cli
1:	hlt
	jmp 1b