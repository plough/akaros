/* See COPYRIGHT for copyright information. */

#ifndef ROS_KERN_ENV_H
#define ROS_KERN_ENV_H
/* Note that the old include/ros/env.h is merged into this file */

#include <ros/memlayout.h>
#include <ros/syscall.h>
#include <ros/sysevent.h>
#include <ros/error.h>
#include <arch/trap.h>
#include <arch/types.h>
#include <arch/arch.h>
#include <sys/queue.h>

struct Env;
typedef struct Env env_t;

typedef int32_t envid_t;

// An environment ID 'envid_t' has three parts:
//
// +1+---------------21-----------------+--------10--------+
// |0|          Uniqueifier             |   Environment    |
// | |                                  |      Index       |
// +------------------------------------+------------------+
//                                       \--- ENVX(eid) --/
//
// The environment index ENVX(eid) equals the environment's offset in the
// 'envs[]' array.  The uniqueifier distinguishes environments that were
// created at different times, but share the same environment index.
//
// All real environments are greater than 0 (so the sign bit is zero).
// envid_ts less than 0 signify errors.  The envid_t == 0 is special, and
// stands for the current environment.

#define LOG2NENV		10
#define NENV			(1 << LOG2NENV)
#define ENVX(envid)		((envid) & (NENV - 1))

// TODO: clean this up.
struct Env {
	LIST_ENTRY(Env) env_link NOINIT;	// Free list link pointers
	uint32_t lock;
	trapframe_t env_tf 						// Saved registers
	  __attribute__((aligned (8)));			// for sparc --asw
	ancillary_state_t env_ancillary_state 	// State saved when descheduled
	  __attribute__((aligned (8)));			// for sparc --asw
	envid_t env_id;				// Unique environment identifier
	envid_t env_parent_id;		// env_id of this env's parent
	uint32_t state;				// Status of the process
	uint32_t env_runs;			// Number of times environment has run
	uint32_t env_refcnt;		// Reference count of kernel contexts using this
	uint32_t env_flags;
	// The backring for processing asynchronous system calls from the user
	// Note this is the actual backring, not a pointer to it somewhere else
	syscall_back_ring_t env_syscallbackring;
	// The front ring for pushing asynchronous system events out to the user
	// Note this is the actual frontring, not a pointer to it somewhere else
	sysevent_front_ring_t env_syseventfrontring;

	// Address space
	pde_t *COUNT(NPDENTRIES) env_pgdir;			// Kernel virtual address of page dir
	physaddr_t env_cr3;			// Physical address of page dir

	// TODO - give this a proper type (pointers to a struct)
 	// - not always going to be PGSIZE either!
 	void*COUNT(PGSIZE) env_procinfo; 		// KVA of per-process shared info table (RO)
	// TODO - do we really want to get rid of procdata?
	//void*COUNT(PGSIZE) env_procdata; 		// KVA of per-process shared data table (RW)
	
	// Do we need these and the above back/front rings?
	// Ring buffers for communicating with user space
	syscall_sring_t*SAFE env_syscallring;   // Per-process ring buffer for async syscalls
	sysevent_sring_t*SAFE env_syseventring; // Per-process ring buffer for async sysevents
};

/* Process Flags */
// None yet

extern env_t *COUNT(NENV) envs;		// All environments
extern atomic_t num_envs;		// Number of envs
extern env_t* NORACE curenvs[MAX_NUM_CPUS];

LIST_HEAD(env_list, Env);		// Declares 'struct env_list'
typedef struct env_list env_list_t;

void	env_init(void);
int		env_alloc(env_t *SAFE*SAFE e, envid_t parent_id);
void	env_init_trapframe(env_t* e);
void	env_set_program_counter(env_t* e, uintptr_t pc);
void	env_push_ancillary_state(env_t* e);
void	env_pop_ancillary_state(env_t* e);
void	env_free(env_t *SAFE e);
void	env_user_mem_free(env_t* e);
error_t	env_incref(env_t* e);
void	env_decref(env_t *SAFE e);
env_t*	env_create(uint8_t *COUNT(size) binary, size_t size);
void	(IN_HANDLER env_destroy)(env_t *SAFE e);	// Does not return if e == curenv
// Temporary scheduler function
void	schedule(void);

/*
 * Allows the kernel to figure out what process is running on its core.
 * Can be used just like a pointer to a struct process.
 */
#define current (curenvs[core_id()])

int	envid2env(envid_t envid, env_t **env_store, bool checkperm);
// The following three functions do not return
void	(IN_HANDLER env_run)(env_t *e) __attribute__((noreturn));
void	env_pop_tf(trapframe_t *tf) __attribute__((noreturn));


/* Helper handler for smp_call to dispatch jobs to other cores */
void run_env_handler(trapframe_t *tf, void* data);

// TODO remove this legacy crap
#define ENV_CREATE(x)			({                                             \
	extern uint8_t _binary_obj_user_apps_##x##_start[],                        \
		_binary_obj_user_apps_##x##_size[];                                    \
	env_t *e = env_create(_binary_obj_user_apps_##x##_start,                   \
		(int)_binary_obj_user_apps_##x##_size);                                \
	proc_set_state(e, PROC_RUNNABLE_S);                                        \
	e;                                                                         \
})

#endif // !ROS_KERN_ENV_H