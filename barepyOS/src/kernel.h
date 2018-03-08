#ifndef _KERNEL_H
#define _KERNEL_H

extern uint32_t CORE0_READY;
extern uint32_t CORE1_READY;
extern uint32_t CORE2_READY;
extern uint32_t CORE3_READY;

extern uint32_t __stack_core0_start__;
extern uint32_t __stacks_core0_end__;

extern uint32_t __svc_stack_core1__;

extern uint32_t __kernel_heap_start__;
extern uint32_t __kernel_heap_end__;


#endif
