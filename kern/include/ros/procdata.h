/* Copyright (c) 2009 The Regents of the University of California
 * See LICENSE for details.  */

#ifndef ROS_PROCDATA_H
#define ROS_PROCDATA_H

#include <ros/memlayout.h>
#include <ros/ring_syscall.h>
#include <ros/sysevent.h>
#include <ros/arch/arch.h>
#include <ros/common.h>
#include <ros/procinfo.h>
#include <ros/event.h>

typedef struct procdata {
	/*
	syscall_sring_t			syscallring;
	char					pad1[SYSCALLRINGSIZE - sizeof(syscall_sring_t)];
	*/
	syscall_sring_t			*syscallring;
	sysevent_sring_t		syseventring;
	char					pad2[SYSEVENTRINGSIZE - sizeof(sysevent_sring_t)];
#if defined (__i386__) || defined (__x86_64) /* TODO: 64b */
	segdesc_t				*ldt; // TODO: bug with this. (TLSV)
#endif
	/* glibc relies on stuff above this point.  if you change it, you need to
	 * rebuild glibc. */
	struct resource_req		res_req[MAX_NUM_RESOURCES];
	struct event_queue		*kernel_evts[MAX_NR_EVENT];
	/* Long range, would like these to be mapped in lazily, as the vcores are
	 * requested.  Sharing MAX_NUM_CPUS is a bit weird too. */
	struct preempt_data		vcore_preempt_data[MAX_NUM_CPUS];
} procdata_t;

#define PROCDATA_NUM_PAGES  ((sizeof(procdata_t)-1)/PGSIZE + 1)

/* TODO: I dislike having this not be a pointer (for kernel programming) */
#define __procdata (*(procdata_t*)UDATA)

#endif /* ROS_PROCDATA_H */
