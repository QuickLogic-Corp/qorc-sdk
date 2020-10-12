/* Internal Memory Map*/
ENTRY(Reset_Handler)
MEMORY
{
	rom (rx)  : ORIGIN = 0x20000000, LENGTH = 0x0000A000
	ram (rwx) : ORIGIN = 0x2000A000, LENGTH = 0x00006000
	stack	  : ORIGIN = 0x2007E000, LENGTH = 0x00001000
	LOADER (rx)	  : ORIGIN = 0x2007F200, LENGTH = 0x00000E00
}

SECTIONS
{
	.text :
	{
		KEEP(*(.isr_vector))
		
		KEEP(*(.init))
		KEEP(*(.fini))
		
		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)
		
		/* .dtors */
		*crtbegin.o(.dtors)
		*crtbegin?.o(.dtors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
		*(SORT(.dtors.*))
		*(.dtors)
		
		*(.rodata*)
		
		KEEP(*(.eh_fram e*))
	} > rom 

	/* To copy multiple ROM to RAM sections,
	 * uncomment .copy.table section and,
	 * define __STARTUP_COPY_MULTIPLE in startup_ARMCMx.S */
	
    .copy.table :
    {
        . = ALIGN(4);
        __copy_table_start__ = .;
        LONG (__etext)
        LONG (__data_start__)
        LONG (__data_end__ - __data_start__)
        LONG (__LOADER_FUNC_load__)
        LONG (__LOADER_FUNC_start__)
        LONG (__LOADER_FUNC_end__ - __LOADER_FUNC_start__)
        __copy_table_end__ = .;
    } > rom

	.LOADER_FUNC :
	{
	. = ALIGN(4);
	__LOADER_FUNC_load__ = LOADADDR(.LOADER_FUNC);
	__LOADER_FUNC_start__ = .;
		*(.LOADER_FUNC*);		
    . = ALIGN(4);
    __LOADER_FUNC_end__ = .;
   	} > LOADER AT > rom
	
	.txt :
	{
		*(.text*)
	} > rom
	
	.ARM.extab : 
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > rom 
	
	__exidx_start = .;
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > rom 
	__exidx_end = .;
	__etext = .;
	
	.data : 
	{
		__data_start__ = .;
		
		*(vtable)
		*(.data*)
		
		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
		
		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);
		
		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);
		
		KEEP(*(.jcr*))
		. = ALIGN(4);
		/* All data end */
		__data_end__ = .;
		
	} > ram AT > rom 

	.bss :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		_sbss = __bss_start__;
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
		_ebss = __bss_end__;
	} > ram 
		
    .rtos :
    {
        . = ALIGN(4);
        __rtos_start__ = .;
        *(.rtos*)
        . = ALIGN(4);
        __rtos_end__ = .;
        _ertos = __rtos_end__;
    } > ram

   
	.heap (COPY):
	{
		__end__ = .;
		_end = __end__;
		end = __end__;
		__HeapBase = _end;
		*(.heap*)
		__HeapLimit = .;
	} > ram 
	
	/* .stack_dummy section doesn't contains any symbols. It is only
	* used for linker to calculate size of stack sections, and assign
	* values to stack symbols later */
	.co_stack (NOLOAD):
	{
		. = ALIGN(8);
		*(.co_stack .co_stack.*)
	} > ram 
	
	/* Set stack top to end of ram , and stack limit move down by
	* size of stack_dummy section */
	__StackTop = ORIGIN(stack) + LENGTH(stack );
	__StackLimit = __StackTop - LENGTH(stack );
	PROVIDE(__stack = __StackTop);
	
	/* Check if data + heap + stack exceeds ram  limit */
	ASSERT(__StackLimit >= __HeapLimit, "region ram  overflowed with stack") 
}
