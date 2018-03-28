
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

  ldr r2, = __irq_stack_core0__
  ldr r3, = __svc_stack_core0__
  ldr r4, = __sys_stack_core0__
  mrc p15, 0, r5, c0, c0, 5       ;@ read core affinity register
  ands r5, r5, #0x3               ;@ core id bitmask
  cmp r5, #0
  beq set_stacks
  ldr r2, = __irq_stack_core1__
  ldr r3, = __svc_stack_core1__
  ldr r4, = __sys_stack_core1__
  cmp r5, #1
  beq set_stacks
  ldr r2, = __irq_stack_core2__
  ldr r3, = __svc_stack_core2__
  ldr r4, = __sys_stack_core2__
  cmp r5, #2
  ldr r2, = __irq_stack_core3__
  ldr r3, = __svc_stack_core3__
  ldr r4, = __sys_stack_core3__
  cmp r5, #3

set_stacks:
  ;@ r3 address of the irq_stack
  ;@ r4 address of the svc_stack
  ;@ r5 address of the sys_stack
  ;@ FIQ
  ;@ (PSR_FIQ_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
  mov r0,#0xD1
  msr cpsr_c,r0
  mov sp, r3
  ;@ ldr sp, =__irq_stack_end__

  ;@ IRQ
  ;@ (PSR_IRQ_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
  mov r0,#0xD2
  msr cpsr_c,r0
  mov sp, r3
  ;@ ldr sp, =__irq_stack_end__

 ;@ (PSR_ABORT_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
  mov r0,#0xD7
  msr cpsr_c,r0
  mov sp, r3
  ;@ ldr sp, =__irq_stack_end__

  ;@ (PSR_SVC_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
  mov r0,#0xD3
  msr cpsr_c,r0
  mov sp, r4
  ;@ ldr sp, =__svc_stack_end__

  ;@ (PSR_SYSTEM_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
  mov r0,#0xDF
  msr cpsr_c,r0
  mov sp, r5
  ;@ ldr sp, =__sys_stack_end__

  mrc p15, 0, r5, c0, c0, 5
  ands r5, r5, #0x3
  cmp r5, #0
  beq .exit_park ;@ core0 exit park
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

  ldr r0, =__bss_start__
	ldr r1, =__bss_end__
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
  CLREX
  b disjoin_smp

.core2_ack:
  ldr r1, =CORE2_READY
  ldr r0, [r1]
  mov r0, #1
  str r0, [r1]
  b disjoin_smp

.core3_ack:
  ldr r1, =CORE3_READY
  ldr r0, [r1]
  mov r0, #1
  str r0, [r1]
  b disjoin_smp

disjoin_smp:
  clrex
  mrc p15, 0, r0, c1, c0, 1 // read ACTLR
  bic r0, #0x040 // clear bit 6 (SMP) to 1
  mcr p15, 0, r0, c1, c0, 1 // write ACTLR
  isb // ensure all the cp15 changes are commited
  dsb // ensure all cache, tlb, tlb, branch prediction
  wfi // set processor in idle low power state
  b halt // infinite loop


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
  swi_vector:        .word swi_handler
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
	b swi_handler

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
