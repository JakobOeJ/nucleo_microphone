/* _start_addr is used to set vector table offset at runtime */
_start_addr = LDADDR;
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = 0x20017000;    /* end of RAM */

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x200;      /* required amount of heap  */
_Min_Stack_Size = 0x400; /* required amount of stack */


/* Specify the memory areas */
MEMORY
{
	FLASH    (xrw)    : ORIGIN = 0x08000000, LENGTH = 512K
	FIRMWARE (xrw)    : ORIGIN = LDADDR,     LENGTH = 128K
	RAM      (xrw)    : ORIGIN = 0x20000000, LENGTH =  96K
	CCMRAM    (rw)    : ORIGIN = 0x10000000, LENGTH =  64K
}


/* Define output sections */
SECTIONS
{
	/* The startup code goes first into FIRMWARE */
	.isr_vector :
	{
		. = ALIGN(4);
		KEEP(*(.isr_vector)) /* Startup code */
		. = ALIGN(4);
	} >FIRMWARE

	/* The program code and other data goes into FIRMWARE */
	.text :
	{
		. = ALIGN(4);
		*(.text)           /* .text sections (code) */
		*(.text*)          /* .text* sections (code) */
		*(.glue_7)         /* glue arm to thumb code */
		*(.glue_7t)        /* glue thumb to arm code */
		*(.eh_frame)

		KEEP (*(.init))
		KEEP (*(.fini))

		. = ALIGN(4);
		_etext = .;        /* define a global symbols at end of code */
	} >FIRMWARE

	/* Constant data goes into FIRMWARE */
	.rodata :
	{
		. = ALIGN(4);
		*(.rodata)         /* .rodata sections (constants, strings, etc.) */
		*(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
		. = ALIGN(4);
	} >FIRMWARE

	.ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FIRMWARE
	.ARM : {
	__exidx_start = .;
	*(.ARM.exidx*)
	__exidx_end = .;
	} >FIRMWARE

	.preinit_array     :
	{
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP (*(.preinit_array*))
		PROVIDE_HIDDEN (__preinit_array_end = .);
	} >FIRMWARE
	.init_array :
	{
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array*))
		PROVIDE_HIDDEN (__init_array_end = .);
	} >FIRMWARE
	.fini_array :
	{
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT(.fini_array.*)))
		KEEP (*(.fini_array*))
		PROVIDE_HIDDEN (__fini_array_end = .);
	} >FIRMWARE

	/* used by the startup to initialize data */
	_sidata = LOADADDR(.data);

	/* Initialized data sections goes into RAM, load LMA copy after code */
	.data :
	{
		. = ALIGN(4);
		_sdata = .;        /* create a global symbol at data start */
		*(.data)           /* .data sections */
		*(.data*)          /* .data* sections */

		. = ALIGN(4);
		_edata = .;        /* define a global symbol at data end */
	} >RAM AT> FIRMWARE

	_siccmram = LOADADDR(.ccmram);

	/* CCM-RAM section
	*
	* IMPORTANT NOTE!
	* If initialized variables will be placed in this section,
	* the startup code needs to be modified to copy the init-values.
	*/
	.ccmram :
	{
		. = ALIGN(4);
		_sccmram = .;       /* create a global symbol at ccmram start */
		*(.ccmram)
		*(.ccmram*)

		. = ALIGN(4);
		_eccmram = .;       /* create a global symbol at ccmram end */
	} >CCMRAM AT> FIRMWARE


	/* Uninitialized data section */
	. = ALIGN(4);
	.bss :
	{
		/* This is used by the startup in order to initialize the .bss secion */
		_sbss = .;         /* define a global symbol at bss start */
		__bss_start__ = _sbss;
		*(.bss)
		*(.bss*)
		*(COMMON)

		. = ALIGN(4);
		_ebss = .;         /* define a global symbol at bss end */
		__bss_end__ = _ebss;
	} >RAM

	/* User_heap_stack section, used to check that there is enough RAM left */
	._user_heap_stack :
	{
		. = ALIGN(4);
		PROVIDE ( end = . );
		PROVIDE ( _end = . );
		PROVIDE ( __end__ = . );
		. = . + _Min_Heap_Size;
		. = . + _Min_Stack_Size;
		. = ALIGN(4);
	} >RAM

	/* Remove information from the standard libraries */
	/DISCARD/ :
	{
		libc.a ( * )
		libm.a ( * )
		libgcc.a ( * )
	}

	.ARM.attributes 0 : { *(.ARM.attributes) }
}