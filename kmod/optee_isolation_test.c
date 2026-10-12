// SPDX-License-Identifier: GPL-2.0
/*
 * Debug-only: from S-mode (Linux) try to read the OP-TEE secure RAM and report
 * whether OpenSBI's PMP blocks it. Out-of-tree; never shipped in the image.
 */
#include <linux/module.h>
#include <linux/io.h>
#include <linux/uaccess.h>

static unsigned long base = 0x36000000UL;
module_param(base, ulong, 0444);
MODULE_PARM_DESC(base, "physical base of the OP-TEE reserved region");

static int __init isol_init(void)
{
	void __iomem *va;
	u32 val;
	int ret;

	va = ioremap(base, PAGE_SIZE);
	if (!va) {
		pr_err("optee_isol: ioremap(0x%lx) failed\n", base);
		return -ENOMEM;
	}

	/* The fault is trapped by the exception table, not fatal. */
	ret = copy_from_kernel_nofault(&val, (void *)va, sizeof(val));
	iounmap(va);

	if (ret) {
		pr_info("optee_isol: PASS -- read of 0x%lx faulted, TEE RAM is isolated from S-mode\n",
			base);
	} else {
		pr_warn("optee_isol: LEAK -- read of 0x%lx returned 0x%08x, TEE RAM is NOT isolated\n",
			base, val);
	}
	/* Nothing to keep resident; refuse to stay loaded. */
	return -ECANCELED;
}
module_init(isol_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("OP-TEE S-mode isolation self-test (debug)");
