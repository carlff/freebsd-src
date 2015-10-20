/*-
 * SPDX-License-Identifier: MIT OR GPL-2.0-only
 *
 * Copyright © 2015 Julien Grall
 * Copyright © 2013 Spectra Logic Corporation
 * Copyright © 2018 John Baldwin/The FreeBSD Foundation
 * Copyright © 2019 Roger Pau Monné/Citrix Systems R&D
 * Copyright © 2021 Elliott Mitchell
 *
 * This file may be distributed separately from the Linux kernel, or
 * incorporated into other software packages, subject to the following license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this source file (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/limits.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/interrupt.h>
#include <sys/pcpu.h>
#include <sys/proc.h>
#include <sys/smp.h>
#include <sys/stddef.h>

#include <xen/xen-os.h>
#include <xen/xen_intr.h>
#include <machine/xen/arch-intr.h>

#include <x86/apicvar.h>

/************************ Xen x86 interrupt interface ************************/

/*
 * Pointers to the interrupt counters
 */
DPCPU_DEFINE_STATIC(u_long *, pintrcnt);

static void
xen_intrcnt_init(void *dummy __unused)
{
	unsigned int i;

	if (!xen_domain())
		return;

	CPU_FOREACH(i) {
		char buf[MAXCOMLEN + 1];

		snprintf(buf, sizeof(buf), "cpu%d:xen", i);
		intrcnt_add(buf, DPCPU_ID_PTR(i, pintrcnt));
	}
}
SYSINIT(xen_intrcnt_init, SI_SUB_INTR, SI_ORDER_MIDDLE, xen_intrcnt_init, NULL);

/*
 * Transition from assembly language, called from
 * sys/{amd64/amd64|i386/i386}/apic_vector.S
 */
extern void xen_arch_intr_handle_upcall(struct trapframe *);
void
xen_arch_intr_handle_upcall(struct trapframe *trap_frame)
{
	struct trapframe *old;

	/*
	 * Disable preemption in order to always check and fire events
	 * on the right vCPU
	 */
	critical_enter();

	++*DPCPU_GET(pintrcnt);

	++curthread->td_intr_nesting_level;
	old = curthread->td_intr_frame;
	curthread->td_intr_frame = trap_frame;

	xen_intr_handle_upcall(NULL);

	curthread->td_intr_frame = old;
	--curthread->td_intr_nesting_level;

	if (xen_evtchn_needs_ack)
		lapic_eoi();

	critical_exit();
}

/******************************** EVTCHN PIC *********************************/

static void
xen_intr_pic_enable_source(struct intsrc *isrc)
{

	_Static_assert(offsetof(struct xenisrc, xi_arch.intsrc) == 0,
	    "xi_arch MUST be at top of xenisrc for x86");
	xen_intr_enable_source((struct xenisrc *)isrc);
}

/*
 * Perform any necessary end-of-interrupt acknowledgements.
 *
 * \param isrc  The interrupt source to EOI.
 */
static void
xen_intr_pic_disable_source(struct intsrc *isrc, int eoi)
{

	_Static_assert(offsetof(struct xenisrc, xi_arch.intsrc) == 0,
	    "xi_arch MUST be at top of xenisrc for x86");
	xen_intr_disable_source((struct xenisrc *)isrc);
}

static void
xen_intr_pic_eoi_source(struct intsrc *isrc)
{

	/* Nothing to do on end-of-interrupt */
}

static void
xen_intr_pic_enable_intr(struct intsrc *isrc)
{

	_Static_assert(offsetof(struct xenisrc, xi_arch.intsrc) == 0,
	    "xi_arch MUST be at top of xenisrc for x86");
	xen_intr_enable_intr((struct xenisrc *)isrc);
}

static void
xen_intr_pic_disable_intr(struct intsrc *isrc)
{

	_Static_assert(offsetof(struct xenisrc, xi_arch.intsrc) == 0,
	    "xi_arch MUST be at top of xenisrc for x86");
	xen_intr_disable_intr((struct xenisrc *)isrc);
}

/**
 * Determine the global interrupt vector number for
 * a Xen interrupt source.
 *
 * \param isrc  The interrupt source to query.
 *
 * \return  The vector number corresponding to the given interrupt source.
 */
static int
xen_intr_pic_vector(struct intsrc *isrc)
{

	_Static_assert(offsetof(struct xenisrc, xi_arch.intsrc) == 0,
	    "xi_arch MUST be at top of xenisrc for x86");

	return (((struct xenisrc *)isrc)->xi_arch.vector);
}

/**
 * Determine whether or not interrupt events are pending on the
 * the given interrupt source.
 *
 * \param isrc  The interrupt source to query.
 *
 * \returns  0 if no events are pending, otherwise non-zero.
 */
static int
xen_intr_pic_source_pending(struct intsrc *isrc)
{
	/*
	 * EventChannels are edge triggered and never masked.
	 * There can be no pending events.
	 */
	return (0);
}

/**
 * Prepare this PIC for system suspension.
 */
static void
xen_intr_pic_suspend(struct pic *pic)
{

	/* Nothing to do on suspend */
}

static void
xen_intr_pic_resume(struct pic *pic, bool suspend_cancelled)
{

	if (!suspend_cancelled)
		xen_intr_resume();
}

/**
 * Perform configuration of an interrupt source.
 *
 * \param isrc  The interrupt source to configure.
 * \param trig  Edge or level.
 * \param pol   Active high or low.
 *
 * \returns  0 if no events are pending, otherwise non-zero.
 */
static int
xen_intr_pic_config_intr(struct intsrc *isrc, enum intr_trigger trig,
    enum intr_polarity pol)
{
	/* Configuration is only possible via the evtchn apis. */
	return (ENODEV);
}


static int
xen_intr_pic_assign_cpu(struct intsrc *isrc, u_int apic_id)
{

	_Static_assert(offsetof(struct xenisrc, xi_arch.intsrc) == 0,
	    "xi_arch MUST be at top of xenisrc for x86");
	return (xen_intr_assign_cpu((struct xenisrc *)isrc,
	    apic_cpuid(apic_id)));
}

/**
 * PIC interface for all event channel port types except physical IRQs.
 */
struct pic xen_intr_pic = {
	.pic_enable_source  = xen_intr_pic_enable_source,
	.pic_disable_source = xen_intr_pic_disable_source,
	.pic_eoi_source     = xen_intr_pic_eoi_source,
	.pic_enable_intr    = xen_intr_pic_enable_intr,
	.pic_disable_intr   = xen_intr_pic_disable_intr,
	.pic_vector         = xen_intr_pic_vector,
	.pic_source_pending = xen_intr_pic_source_pending,
	.pic_suspend        = xen_intr_pic_suspend,
	.pic_resume         = xen_intr_pic_resume,
	.pic_config_intr    = xen_intr_pic_config_intr,
	.pic_assign_cpu     = xen_intr_pic_assign_cpu,
};

/******************************* ARCH wrappers *******************************/

void
xen_arch_intr_init(void)
{
	int error;

	error = intr_register_pic(&xen_intr_pic);
	if (error != 0)
		panic("%s(): failed registering Xen/x86 PIC, error=%d\n",
		    __func__, error);
}
