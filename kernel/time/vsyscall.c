// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 ARM Ltd.
 *
 * Generic implementation of update_vsyscall and update_vsyscall_tz.
 *
 * Based on the x86 specific implementation.
 */

#include <linux/hrtimer.h>
#include <linux/timekeeper_internal.h>
#include <vdso/datapage.h>
#include <vdso/helpers.h>
#include <vdso/vsyscall.h>
#include <asm/syscall.h>
#include <linux/sched.h>

#include "timekeeping_internal.h"


void update_vdso_data(void)
{
	
	struct vdso_data *vdata = __arch_get_k_vdso_data();
	struct timekeeper *tk = NULL;
	/* copy vsyscall data */
	vdso_write_begin(vdata);

	vdata[CS_HRES_COARSE].sys_call_table = sys_call_table;
	vdata[CS_HRES_COARSE].kernel_sched_timeout = (unsigned long *) schedule_timeout;

	vdata[CS_RAW].sys_call_table = sys_call_table;
	vdata[CS_RAW].kernel_sched_timeout = (unsigned long *)schedule_timeout;


	__arch_update_vsyscall(vdata, tk);

	vdso_write_end(vdata);

	__arch_sync_vdso_data(vdata);
}

/**
 * vdso_update_begin - Start of a VDSO update section
 *
 * Allows architecture code to safely update the architecture specific VDSO
 * data. Disables interrupts, acquires timekeeper lock to serialize against
 * concurrent updates from timekeeping and invalidates the VDSO data
 * sequence counter to prevent concurrent readers from accessing
 * inconsistent data.
 *
 * Returns: Saved interrupt flags which need to be handed in to
 * vdso_update_end().
 */
unsigned long vdso_update_begin(void)
{
	struct vdso_data *vdata = __arch_get_k_vdso_data();
	unsigned long flags;

	raw_spin_lock_irqsave(&timekeeper_lock, flags);
	vdso_write_begin(vdata);
	return flags;
}

/**
 * vdso_update_end - End of a VDSO update section
 * @flags:	Interrupt flags as returned from vdso_update_begin()
 *
 * Pairs with vdso_update_begin(). Marks vdso data consistent, invokes data
 * synchronization if the architecture requires it, drops timekeeper lock
 * and restores interrupt flags.
 */
void vdso_update_end(unsigned long flags)
{
	struct vdso_data *vdata = __arch_get_k_vdso_data();

	vdso_write_end(vdata);
	__arch_sync_vdso_data(vdata);
	raw_spin_unlock_irqrestore(&timekeeper_lock, flags);
}
