/*
 * Up to Android O the arm32 libc exported the linker-defined bounds of its
 * .ARM.exidx table; libfastcvopt.so (and with it the camera daemon and
 * libmmjpeg) still imports __exidx_start/__exidx_end. Provide an empty
 * range: the blob only needs the symbols to resolve.
 */
__asm__(
    "    .data\n"
    "    .balign 4\n"
    "    .globl __exidx_start\n"
    "    .type __exidx_start, %object\n"
    "    .globl __exidx_end\n"
    "    .type __exidx_end, %object\n"
    "__exidx_start:\n"
    "__exidx_end:\n"
    "    .word 0\n");
