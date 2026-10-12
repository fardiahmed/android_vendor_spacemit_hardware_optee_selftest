// SPDX-License-Identifier: BSD-2-Clause
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <selftest_ta.h>

/* Persistent object seals with the per-device key: the on-disk blob in the REE
 * filesystem is ciphertext, the plaintext only ever exists inside the TA. */

TEE_Result TA_CreateEntryPoint(void) { return TEE_SUCCESS; }
void TA_DestroyEntryPoint(void) {}

TEE_Result TA_OpenSessionEntryPoint(uint32_t pt, TEE_Param params[4] __unused,
				    void **sess __unused)
{
	if (pt != TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
				  TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE))
		return TEE_ERROR_BAD_PARAMETERS;
	IMSG("optee_selftest: session opened");
	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *sess __unused) {}

static TEE_Result cmd_seal(uint32_t pt, TEE_Param params[4])
{
	const uint32_t flags = TEE_DATA_FLAG_ACCESS_WRITE |
			       TEE_DATA_FLAG_OVERWRITE;
	TEE_ObjectHandle obj = TEE_HANDLE_NULL;
	TEE_Result res;

	if (pt != TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				  TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
				  TEE_PARAM_TYPE_NONE))
		return TEE_ERROR_BAD_PARAMETERS;
	if (params[0].memref.size > SELFTEST_MAX_PAYLOAD)
		return TEE_ERROR_EXCESS_DATA;

	res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, SELFTEST_OBJ_ID,
					 SELFTEST_OBJ_ID_LEN, flags,
					 TEE_HANDLE_NULL, params[0].memref.buffer,
					 params[0].memref.size, &obj);
	if (res != TEE_SUCCESS)
		return res;
	TEE_CloseObject(obj);
	IMSG("optee_selftest: sealed %u bytes", params[0].memref.size);
	return TEE_SUCCESS;
}

static TEE_Result cmd_unseal(uint32_t pt, TEE_Param params[4])
{
	TEE_ObjectHandle obj = TEE_HANDLE_NULL;
	uint32_t got = 0;
	TEE_Result res;

	if (pt != TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
				  TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
				  TEE_PARAM_TYPE_NONE))
		return TEE_ERROR_BAD_PARAMETERS;

	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, SELFTEST_OBJ_ID,
				       SELFTEST_OBJ_ID_LEN,
				       TEE_DATA_FLAG_ACCESS_READ, &obj);
	if (res != TEE_SUCCESS)
		return res;

	res = TEE_ReadObjectData(obj, params[0].memref.buffer,
				 params[0].memref.size, &got);
	TEE_CloseObject(obj);
	if (res != TEE_SUCCESS)
		return res;
	params[0].memref.size = got;
	IMSG("optee_selftest: unsealed %u bytes", got);
	return TEE_SUCCESS;
}

static TEE_Result cmd_wipe(uint32_t pt, TEE_Param params[4] __unused)
{
	TEE_ObjectHandle obj = TEE_HANDLE_NULL;
	TEE_Result res;

	if (pt != TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
				  TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE))
		return TEE_ERROR_BAD_PARAMETERS;

	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, SELFTEST_OBJ_ID,
				       SELFTEST_OBJ_ID_LEN,
				       TEE_DATA_FLAG_ACCESS_WRITE_META, &obj);
	if (res != TEE_SUCCESS)
		return res;
	TEE_CloseAndDeletePersistentObject1(obj);
	return TEE_SUCCESS;
}

TEE_Result TA_InvokeCommandEntryPoint(void *sess __unused, uint32_t cmd,
				      uint32_t pt, TEE_Param params[4])
{
	switch (cmd) {
	case TA_SELFTEST_CMD_SEAL:
		return cmd_seal(pt, params);
	case TA_SELFTEST_CMD_UNSEAL:
		return cmd_unseal(pt, params);
	case TA_SELFTEST_CMD_WIPE:
		return cmd_wipe(pt, params);
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
