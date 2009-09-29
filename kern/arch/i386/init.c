/* See COPYRIGHT for copyright information. */

#ifdef __SHARC__
#pragma nosharc
#endif

#include <smp.h>

#include <arch/rl8168.h>
#include <arch/ne2k.h>
#include <arch/mptables.h>
#include <arch/pci.h>
#include <arch/ioapic.h>

void arch_init()
{
	mptables_parse();
	pci_init();
	ioapic_init(); // MUST BE AFTER PCI/ISA INIT!
		
	// this returns when all other cores are done and ready to receive IPIs
	smp_boot();
	env_init();

	/* EXPERIMENTAL NETWORK FUNCTIONALITY
	 * To enable, define __NETWORK__ in your Makelocal
	 * If enabled, will load the rl8168 driver (if device exists)
	 * and will a boot into userland matrix, so remote syscalls can be performed.
 	 * If in simulation, will do some debugging information with the ne2k device
	 *
	 * Note: If you use this, you should also define the mac address of the 
	 * teathered machine via USER_MAC_ADDRESS in Makelocal.
	 *
	 * Additionally, you should have a look at the syscall server in the tools directory
	 */
	#ifdef __NETWORK__
	rl8168_init();		
	ne2k_init();
	#endif // __NETWORK__
}