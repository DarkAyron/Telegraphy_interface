/*                  ___====-_  _-====___
              _--^^^#####//      \\#####^^^--_
           _-^##########// (    ) \\##########^-_
          -############//  |\^^/|  \\############-
        _/############//   (@::@)   \\############\_
       /#############((     \\//     ))#############\
      -###############\\    (oo)    //###############-
     -#################\\  / VV \  //#################-
    -###################\\/      \//###################-
   _#/|##########/\######(   /\   )######/\##########|\#_
   |/ |#/\#/\#/\/  \#/\##\  |  |  /##/\#/  \/\#/\#/\#| \|
   `  |/  V  V  `   V  \#\| |  | |/#/  V   '  V  V  \|  '
      `   `  `      `   / | |  | | \   '      '  '   '
                       (  | |  | |  )
                      __\ | |  | | /__
                     (vvv(VVV)(VVV)vvv)
 
armelf_sam_eabi.x

Linker script for atsam executables

Copyright (c) 2016 Ayron
*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
	"elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY
{
	flash0 (rx)	: ORIGIN = 0x00000000, LENGTH = 0x00040000
	flash1 (rx)	: ORIGIN = 0x00040000, LENGTH = 0x00040000
	sram0 (rwx)	: ORIGIN = 0x20000000, LENGTH = 0x00010000
	sram1 (rwx)	: ORIGIN = 0x20080000, LENGTH = 0x00008000
}

SECTIONS
{
	.text :
	{
		. = ALIGN(4);
		KEEP(*(.traps*))
		*(.text*)
		KEEP(*(.tables*))
		_etext = .;
	} > flash0
	
	.rodata : AT (_etext)
	{
		*(.rodata*)
		_erodata = .;
	} > flash0
	
	.relocate : AT (_erodata)
	{
		. = ALIGN(4);
		_srelocate = .;
		*(.ramfunc)
		. = ALIGN(4);
		*(.data*)
		. = ALIGN(4);
		_erelocate = .;
	} > sram0
	
	.bss (NOLOAD) :
	{
		. = ALIGN(4);
		*(.bss)
		_ebss = .;
	} > sram0
	
	
}

