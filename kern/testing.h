#ifndef ROS_INC_TESTING_H
#define ROS_INC_TESTING_H

/* This is just a dumping ground for old code used for testing.
 * Someone should go through old commits and bring back other relevant tests.
 * Someone else should actually make these useful on their own
 */

#include <inc/types.h>

#include <kern/trap.h>

void test_ipi_sending(void);
void test_pic_reception(void);
void test_print_info(void);
void test_barrier(void);
void test_interrupts_irqsave(void);
void test_bitmasks(void);

void test_hello_world_handler(struct Trapframe *tf);
void test_print_info_handler(struct Trapframe *tf);
void test_barrier_handler(struct Trapframe *tf);

#endif /* !ROS_INC_TESTING_H */