ENTRY(_start)

SECTIONS
{
    . = 0x80000000 + 0x10000;

    .text BLOCK(4K) : AT(0x10000)
    {
        *(.text.boot)
        *(.text.vector)
        *(.text)
    }

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}
 
	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}
 
	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

    PROVIDE(tiny_kernel_end = .);
}

