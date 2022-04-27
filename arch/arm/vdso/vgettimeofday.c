/*
 * Copyright 2015 Mentor Graphics Corporation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 of the
 * License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/compiler.h>
#include <linux/hrtimer.h>
#include <linux/time.h>
#include <asm/arch_timer.h>
#include <asm/barrier.h>
#include <asm/bug.h>
#include <asm/page.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <asm/vdso_datapage.h>

#ifndef CONFIG_AEABI
#error This code depends on AEABI system call conventions
#endif

extern struct vdso_data *__get_datapage(void);

static notrace u32 __vdso_read_begin(const struct vdso_data *vdata)
{
	u32 seq;
repeat:
	seq = ACCESS_ONCE(vdata->seq_count);
	if (seq & 1) {
		cpu_relax();
		goto repeat;
	}
	return seq;
}

static notrace u32 vdso_read_begin(const struct vdso_data *vdata)
{
	u32 seq;

	seq = __vdso_read_begin(vdata);

	smp_rmb(); /* Pairs with smp_wmb in vdso_write_end */
	return seq;
}

static notrace int vdso_read_retry(const struct vdso_data *vdata, u32 start)
{
	smp_rmb(); /* Pairs with smp_wmb in vdso_write_begin */
	return vdata->seq_count != start;
}

static notrace unsigned long get_syscall_addr(struct vdso_data *vdata, unsigned int num)
{
	u32 seq;
	unsigned long addr;

	if (num >= __NR_syscalls)
		return 0;

	do {
		seq = vdso_read_begin(vdata);
		addr = vdata->sys_call_table[num];
	} while (vdso_read_retry(vdata, seq));

	return addr;

}

int clock_gettime(clockid_t clkid, struct timespec *ts)
__attribute__((weak, alias("__vdso_clock_gettime")));
notrace int __vdso_clock_gettime(clockid_t clkid, struct timespec *ts)
{
	struct vdso_data *vdata;
	int ret = -1;
	unsigned long syscall_addr =0;

	vdata = __get_datapage();
	syscall_addr = get_syscall_addr(vdata, __NR_clock_gettime);
	ret = ((int (*)(clockid_t which_clock, struct timespec *tp))syscall_addr)(
		clkid, ts);

	return ret;
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
__attribute__((weak, alias("__vdso_gettimeofday")));
notrace int __vdso_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	struct vdso_data *vdata;
	int ret;
	unsigned long syscall_addr;

	vdata = __get_datapage();
	syscall_addr = get_syscall_addr(vdata, __NR_gettimeofday);
	ret = ((int (*)(struct timeval *tv, struct timezone *tz))syscall_addr)(
		tv, tz);

	return ret;
}

int sched_yield(void)
__attribute__((weak, alias("__vdso_sched_yield")));
notrace int __vdso_sched_yield(void)
{
	struct vdso_data *vdata;
	int ret;
	unsigned long syscall_addr;

	vdata = __get_datapage();
	syscall_addr = get_syscall_addr(vdata, __NR_sched_yield);
	ret = ((int (*)(void))syscall_addr)();

	return ret;
}

int nanosleep(void)
__attribute__((weak, alias("__vdso_nanosleep")));
notrace int __vdso_nanosleep(const struct timespec *req, struct timespec *rem)
{
	struct vdso_data *vdata;
	int ret;
	unsigned long syscall_addr;

	vdata = __get_datapage();
	syscall_addr = get_syscall_addr(vdata, __NR_nanosleep);
	ret = ((int (*)(const struct timespec *req, struct timespec *rem))syscall_addr)(
		req, rem);

	return ret;
}

signed long kernel_sched_timeout(signed long timeout)
__attribute__((weak, alias("__vdso_kernel_sched_timeout")));
notrace signed long __vdso_kernel_sched_timeout(signed long timeout)
{
	struct vdso_data *vdata;
	int ret;
	unsigned long func_addr;

	vdata = __get_datapage();
	func_addr = vdata->kernel_sched_timeout;

	ret = ((signed long (*)(signed long timeout))func_addr)(timeout);

	return ret;
}

/* Avoid unresolved references emitted by GCC */

void __aeabi_unwind_cpp_pr0(void)
{
}

void __aeabi_unwind_cpp_pr1(void)
{
}

void __aeabi_unwind_cpp_pr2(void)
{
}
