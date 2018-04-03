MEMORY {
	RAM : ORIGIN = 0x8000, LENGTH = 0x20000000-0x8000
    PERIPHERALS : ORIGIN = 0x20000000, LENGTH = 0x20FFFFFF - 0x20000000
}

KERNEL_STACK_SIZE = 512;
SYS_STACK_SIZE = 512;
IRQ_STACK_SIZE = 512;

USER_STACKS_SIZE = 0x100000;

SECTIONS {

	.text : {
	      build/init.o(.text*)
	} > RAM

	.text : {
	      *(.text*)
	} > RAM

     __bss_start__ = .;
    .bss : {
    	 *(.bss*)
    } > RAM
    __bss_end__ = .;

    __rodata_start__ = .;
    .rodata : {
	 *(.rodata*)
    } > RAM
    __rodata_end__ = .;

    __data_start__ = .;
    .data : {
    	 *(.data)
    } > RAM
    __data_end__ = .;

    .stack_core0 : {

       __stack_core1_start__ = .;
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

		.stack_core1 : {

     __stack_core1_start__ = .;
	   . += KERNEL_STACK_SIZE ;
	   . = ALIGN(4);
	   __svc_stack_core1__ = .;

	   . += SYS_STACK_SIZE ;
	   . = ALIGN(4);
	   __sys_stack_core1__ = .;

       . += IRQ_STACK_SIZE ;
	   . = ALIGN(4);
	   __irq_stack_core1__ = .;

   	 __stacks_core1_end__ = .;

    } > RAM

		.stack_core2 : {

     __stack_core2_start__ = .;
	   . += KERNEL_STACK_SIZE ;
	   . = ALIGN(4);
	   __svc_stack_core2__ = .;

	   . += SYS_STACK_SIZE ;
	   . = ALIGN(4);
	   __sys_stack_core2__ = .;

       . += IRQ_STACK_SIZE ;
	   . = ALIGN(4);
	   __irq_stack_core2__ = .;

   	 __stacks_core2_end__ = .;

    } > RAM

		.stack_core3 : {

     __stack_core3_start__ = .;
	   . += KERNEL_STACK_SIZE ;
	   . = ALIGN(4);
	   __svc_stack_core3__ = .;

	   . += SYS_STACK_SIZE ;
	   . = ALIGN(4);
	   __sys_stack_core3__ = .;

       . += IRQ_STACK_SIZE ;
	   . = ALIGN(4);
	   __irq_stack_core3__ = .;

   	 __stacks_core3_end__ = .;

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
