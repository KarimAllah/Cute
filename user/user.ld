/* 
 * Kernel linker script
 *
 * Copyright (C) 2009-2010 Karim Allah Ahmed <karim.allah.ahmed@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2.
 *
 * Refer to official LD documentation "Using LD - the GNU linker"
 * for more information.
 *
 * Symbols prefixed by `__' are used by our C and assembly files;
 * modify them cautiously
 */

OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)

SECTIONS
{
	/* Where the bootsector loads us */
	. = 0x2000000;
	.text : {
		*(.text)
    }
    
    .data : {
    	*(.data)
    }
    .rodata : {
    	*(.rodata)
    }
    
    .bss : {
    	*(.bss)
    }

	/DISCARD/ : {
		*(*)
	}
}
