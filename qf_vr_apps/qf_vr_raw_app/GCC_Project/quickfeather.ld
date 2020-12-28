OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
/* Internal Memory Map*/
ENTRY(Reset_Handler)
/*this is used at multiple locations in the ld file. So change it here only and not directly in the memory section.*/
_ram1_start = 0x2003F000;
MEMORY
{
	rom (rx)    : ORIGIN = 0x00000000, LENGTH = 0x0003F000
	RAM1 (rwx)  : ORIGIN = _ram1_start, LENGTH = 0x0000ef00
	ram (rwx)   : ORIGIN = 0x2004df00, LENGTH = 0x0002df00
	HWA (rw)    : ORIGIN = 0x2007BE00, LENGTH = 0x00000100
    spiram (rw) : ORIGIN = 0x2007C000, LENGTH = 0x00000C00
}

SECTIONS
{
	.text :
	{
		KEEP(*(.isr_vector))
		*(.text*)

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

	/* _sidata is used in coide startup code */
	_sidata = __etext;


	.data : 
	{
		__data_start__ = .;

		/* _sdata is used in coide startup code */
		_sdata = __data_start__;

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

		/* _edata is used in coide startup code */
		_edata = __data_end__;
	} > ram AT > rom

	.Hardware (NOLOAD) :
	{
	  __hwa_start__ = .;  /* create symbol for start of section */
	  *(*HWA*)            /* keep my variable even if not referenced */
	  __hwa_end__ = .;   /* create symbol for end of section */
	} > HWA
	.AUD_DMA_BUFF_LOCATION _ram1_start (NOLOAD) :
    {
        . = ALIGN(4);
        KEEP(*(.AUD_DMA_BUFF_LOCATION)) /* keep it even if not referenced */
    } > RAM1
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

/* SPI shared addresses are fixed: */
    .HOST_SPI_TX_LOCATION 0x2007c000 (NOLOAD) :
    {
        . = ALIGN(4);
        KEEP(*(.HOST_SPI_TX_LOCATION)) /* keep it even if not referenced */
    } > spiram

    .BLE_SPI_TX_LOCATION 0x2007c400 (NOLOAD) :
    {
        . = ALIGN(4);
        KEEP(*(.BLE_SPI_TX_LOCATION)) /* keep it even if not referenced */
    } > spiram
    .BLE_SPI_RX_LOCATION  0x2007c800 (NOLOAD) :
    {
        . = ALIGN(4);
        KEEP(*(.BLE_SPI_RX_LOCATION)) /* keep it even if not referenced */
    } > spiram
	.heap (COPY):
	{
		__end__ = .;
		_end = __end__;
		end = __end__;
		__HeapBase = _end;
		KEEP(*(.heap))
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
	STACK_SIZE = 3K;
	__StackTop = ORIGIN(ram ) + LENGTH(ram );
	__StackLimit = __StackTop - STACK_SIZE;
	PROVIDE(__stack = __StackTop);

	/* Check if data + heap + stack exceeds ram  limit */
	ASSERT(__StackLimit >= __HeapLimit, "region ram  overflowed with stack")
}
