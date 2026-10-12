/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef SELFTEST_TA_H
#define SELFTEST_TA_H

/* OP-TEE isolation self-test TA. Proves secure storage (L1) on the K1. */
#define TA_SELFTEST_UUID \
	{ 0xb8f1e2c4, 0x9a3d, 0x4b6e, \
		{ 0xa1, 0x07, 0x5e, 0x1f, 0x2c, 0x3d, 0x4e, 0x5f } }

#define SELFTEST_OBJ_ID		"spacemit.selftest.vault"
#define SELFTEST_OBJ_ID_LEN	23
#define SELFTEST_MAX_PAYLOAD	1024

/* Seal a payload into TEE private storage (memref[0] in). */
#define TA_SELFTEST_CMD_SEAL	0
/* Unseal and return the payload (memref[0] out). */
#define TA_SELFTEST_CMD_UNSEAL	1
/* Delete the vault object. */
#define TA_SELFTEST_CMD_WIPE	2

#endif /* SELFTEST_TA_H */
