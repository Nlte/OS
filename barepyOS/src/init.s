
;@ global constant
.globl CORE0_READY
CORE0_READY: .4byte 0;
.globl CORE1_READY
CORE1_READY: .4byte 0;
.globl CORE2_READY
CORE2_READY: .4byte 0;
.globl CORE3_READY
CORE3_READY: .4byte 0;

.equ CPU_FIQMODE, 0x11							;@ CPU in FIQ mode
.equ CPU_IRQMODE, 0x12							;@ CPU in IRQ mode
.equ CPU_SVCMODE, 0x13							;@ CPU in SVC mode

;@ I_Bit = 1000 0000					;@ Irq flag bit in cpsr (CPUMODE register)
;@ F_Bit = 0100 0000          ;# Fiq flag bit in cpsr
;@ CPU_FIQMODE_VALUE = cpu in fiq with irq/fiq flags on (disable) = CPU_FIQMODE | I_bit | F_Bit
.equ CPU_FIQMODE_VALUE, 0xD1
.equ CPU_IRQMODE_VALUE, 0xD2
.equ CPU_SVCMODE_VALUE, 0xD3

.section ".text.boot"

.globl _start

;@ Entry point for the kernel
_start:

  ;@ Setup stacks for each core
  ;@ core0
  ldr r2, = __svc_stack_core0__
  ldr r3, = __fiq_stack_core0__
  ldr r4, = __irq_stack_core0__
  mrc p15, 0, r5, c0, c0, 5
  ands r5, r5, #0x3
  cmp r5, #0
  beq set_stacks
  ;@ core1
  ldr r2, = __svc_stack_core1__
  ldr r3, = __fiq_stack_core1__
  ldr r4, = __fiq_stack_core1__
  mrc p15, 0, r5, c0, c0, 5
  ands r5, r5, #0x3
  cmp r5, #1
  beq set_stacks
  ;@ core2
  ldr r2, = __svc_stack_core2__
  ldr r3, = __fiq_stack_core2__
  ldr r4, = __fiq_stack_core2__
  mrc p15, 0, r5, c0, c0, 5
  ands r5, r5, #0x3
  cmp r5, #2
  beq set_stacks
  ;@ core3
  ldr r2, = __svc_stack_core3__
  ldr r3, = __fiq_stack_core3__
  ldr r4, = __fiq_stack_core3__
  mrc p15, 0, r5, c0, c0, 5
  ands r5, r5, #0x3
  cmp r5, #3
  beq set_stacks

set_stacks:
  ;@ SVC mode
  mov sp, r2 ;@ kernel stack
  ;@ FIQ mode
  mov r0, #CPU_FIQMODE_VALUE
  msr cpsr_c, r0
  mov sp, r3
  ;@ IRQ mode
  mov r0, #CPU_IRQMODE_VALUE
  msr cpsr_c, r0
  mov sp, r4
  ;@ all stacks ready go back to SVC mode
  mov r0, #CPU_SVCMODE_VALUE
  msr cpsr_c, r0


  mrc p15, 0, r5, c0, c0, 5
  ands r5, r5, #0x3
  cmp r5, #0
  beq .exit_park ;@ core0 exit park
  mov sp, #0x8000
  b .park  ;@ other cores jump to the park routine

.exit_park:

  ;@ setup stack pointer
  mov sp, #0x8000

  ;@ load IVT at 0x0000

  ldr r0, = _iv_table
  mov r1,#0x0000
  ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
  stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
  ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
  stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}


  ;@ Clear out bss

  ldr r0, =__bss_start
	ldr r1, =__bss_end
	mov r2, #0
	.clear_bss:
    cmp r0, r1
    bge .clear_bss_exit
    str r2, [r0]
      add r0, r0, #4
    b .clear_bss

.clear_bss_exit:


  ;@ wait until all cores ready
  .wait_core1_ack:
    ldr r0, =CORE1_READY
    ldr r1, [r0]
    mov r3, #1
    cmp r1, r3
    bne .wait_core1_ack

  .wait_core2_ack:
    ldr r0, =CORE2_READY
    ldr r1, [r0]
    mov r3, #1
    cmp r1, r3
    bne .wait_core2_ack

  .wait_core3_ack:
    ldr r0, =CORE3_READY
    ldr r1, [r0]
    mov r3, #1
    cmp r1, r3
    bne .wait_core3_ack

  bl kmain

.park:
  mrc p15, 0, r5, c0, c0, 5
  ands r5, r5, #0x3
  cmp r5, #1
  beq .core1_ack
  cmp r5, #2
  beq .core2_ack
  cmp r5, #3
  beq .core3_ack

.core1_ack:
  ldr r1, =CORE1_READY
  ldr r0, [r1]
  mov r0, #1
  str r0, [r1]
  b halt

.core2_ack:
  ldr r1, =CORE2_READY
  ldr r0, [r1]
  mov r0, #1
  str r0, [r1]
  b halt

.core3_ack:
  ldr r1, =CORE3_READY
  ldr r0, [r1]
  mov r0, #1
  str r0, [r1]
  b halt

_iv_table:
      ldr pc,reset_vector
      ldr pc,undefined_vector
      ldr pc,swi_vector
      ldr pc,prefetch_vector
      ldr pc,data_vector
      ldr pc,unused_vector
      ldr pc,irq_vector
      ldr pc,fiq_vector
  reset_vector:      .word reset_asm_handler
  undefined_vector:  .word undefined_asm_handler
  swi_vector:        .word swi_asm_handler
  prefetch_vector:   .word prefetch_asm_handler
  data_vector:       .word data_asm_handler
  unused_vector:     .word unused_asm_handler
  irq_vector:        .word irq_asm_handler
  fiq_vector:        .word fiq_asm_handler

;;@ Trampolines to Interrupt Service Routines

reset_asm_handler:
  b reset_asm_handler

undefined_asm_handler:
	b undefined_asm_handler

swi_asm_handler:
	b swi_asm_handler

prefetch_asm_handler:
	b irq_asm_handler

irq_asm_handler:
	b irq_asm_handler

unused_asm_handler:
	b unused_asm_handler

fiq_asm_handler:
	b fiq_asm_handler

data_asm_handler:
	b data_asm_handler


halt:
	wfe
	b halt
