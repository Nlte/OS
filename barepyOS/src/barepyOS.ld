ENTRY(_start)

MEMORY {
	RAM : ORIGIN = 0x8000, LENGTH = 0x20000000-0x8000
    PERIPHERALS : ORIGIN = 0x20000000, LENGTH = 0x20FFFFFF - 0x20000000
}

KERNEL_STACK_SIZE = 512;
SYS_STACK_SIZE = 512;
IRQ_STACK_SIZE = 512;

SECTIONS
{
    /* Starts at LOADER_ADDR. */
    . = 0x8000;
    __start = .;
    __text_start = .;
    .text : {
	      build/init.o(.text*)
	   } > RAM
    .text :
    {
        KEEP(*(.text.boot))
        *(.text)
    } > RAM
    . = ALIGN(4096); /* align to page size */
    __text_end = .;

    __rodata_start = .;
    .rodata :
    {
        *(.rodata)
    } > RAM
    . = ALIGN(4096); /* align to page size */
    __rodata_end = .;

    __data_start = .;
    .data :
    {
        *(.data)
    } > RAM
    . = ALIGN(4096); /* align to page size */
    __data_end = .;

    __bss_start = .;
    .bss :
    {
        bss = .;
        *(.bss)
    } > RAM
    . = ALIGN(4096); /* align to page size */
    __bss_end = .;
    __end = .;

    .stack : {

       __stack_core0_start__ = .;
	   . += KERNEL_STACK_SIZE ;
	   . = ALIGN(4);
	   __svc_stack_core0__ = .;

	   . += SYS_STACK_SIZE ;
	   . = ALIGN(4);
	   __sys_stack_core0__ = .;

       . += IRQ_STACK_SIZE ;
	   . = ALIGN(4);
	   __irq_stack_core0__ = .;

   	   __stacks_core0_end__ = .;

    } > RAM

    . = ALIGN(4096);
    __kernel_heap_start__ = .;
	. = 0xFFFFFF;
    __kernel_heap_end__ = .;
    . = ALIGN(4096);
    __after_kernel_heap__ = .;

    . = 0x20000000;
    .devices : {
        __devices_start__ = .;
        . += 0xFFFFFF;
        __devices_end__ = .;
    } > PERIPHERALS

}
