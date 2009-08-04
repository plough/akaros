#ifndef ROS_ARCH_SMP_H
#define ROS_ARCH_SMP_H

#include <arch/types.h>
#include <arch/arch.h>
#include <atomic.h>

typedef volatile uint8_t wait_list_t[MAX_NUM_CPUS];

typedef struct
{
	wait_list_t wait_list;
	spinlock_t lock;
} handler_wrapper_t;

#endif