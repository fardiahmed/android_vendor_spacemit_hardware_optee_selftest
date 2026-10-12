// SPDX-License-Identifier: BSD-2-Clause
#include <stdio.h>
#include <string.h>

#include <tee_client_api.h>

#include "selftest_ta.h"

static const char kMarker[] = "spacemit-optee-selftest-secret";

static int fail(const char *what, TEEC_Result r)
{
	fprintf(stderr, "[FAIL] %s: 0x%08x\n", what, r);
	return 1;
}

/* Seal a known marker through the TA, read it back, and compare. A match proves
 * the round trip through OP-TEE private storage; the on-disk blob is ciphertext. */
int main(void)
{
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_SELFTEST_UUID;
	uint32_t origin = 0;
	char out[SELFTEST_MAX_PAYLOAD] = { 0 };
	TEEC_Result res;

	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		return fail("InitializeContext", res);

	res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL,
			       &origin);
	if (res != TEEC_SUCCESS) {
		TEEC_FinalizeContext(&ctx);
		return fail("OpenSession", res);
	}

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);
	op.params[0].tmpref.buffer = (void *)kMarker;
	op.params[0].tmpref.size = sizeof(kMarker);
	res = TEEC_InvokeCommand(&sess, TA_SELFTEST_CMD_SEAL, &op, &origin);
	if (res != TEEC_SUCCESS)
		goto out;
	printf("[L1] sealed %zu bytes into OP-TEE private storage\n",
	       sizeof(kMarker));

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);
	op.params[0].tmpref.buffer = out;
	op.params[0].tmpref.size = sizeof(out);
	res = TEEC_InvokeCommand(&sess, TA_SELFTEST_CMD_UNSEAL, &op, &origin);
	if (res != TEEC_SUCCESS)
		goto out;

	if (op.params[0].tmpref.size == sizeof(kMarker) &&
	    !memcmp(out, kMarker, sizeof(kMarker)))
		printf("[L1] PASS: unsealed payload matches\n");
	else
		printf("[L1] FAIL: unsealed payload differs\n");

out:
	TEEC_InvokeCommand(&sess, TA_SELFTEST_CMD_WIPE, &op, &origin);
	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);
	if (res != TEEC_SUCCESS)
		return fail("InvokeCommand", res);
	return 0;
}
