#include <Engine/IO.h>
#include <System/Log.h>
#include <Runtime/Runtime.h>

#include "D3D12Backend.h"

struct ShaderModuleInfo
{
	uint64_t hash;
	D3D12_SHADER_BYTECODE byteCode;
};

static struct NeArray _modules;

static void Load(const char *path);

void *
Re_ShaderModule(const char *name)
{
	uint64_t hash = Rt_HashString(name);

	ShaderModuleInfo *info = (ShaderModuleInfo *)Rt_ArrayBSearch(&_modules, &hash, Rt_U64CmpFunc);
	if (!info)
		return NULL;

	return &info->byteCode;
}

bool
D3D12Bk_LoadShaders()
{
	if (!Rt_InitArray(&_modules, 10, sizeof(struct ShaderModuleInfo), MH_RenderBackend))
		return false;

	E_ProcessFiles("/Shaders/D3D12", "obj", true, Load);
	Rt_ArraySort(&_modules, Rt_U64CmpFunc);

	return true;
}

void
D3D12Bk_UnloadShaders()
{
	ShaderModuleInfo *info;
	Rt_ArrayForEach(info, &_modules, ShaderModuleInfo *)
		Sys_Free((void *)info->byteCode.pShaderBytecode);
	Rt_TermArray(&_modules);
}

void
Load(const char *path)
{
	ShaderModuleInfo info{};

	NeStream stm{};
	if (!E_FileStream(path, IO_READ, &stm))
		return;

	info.byteCode.BytecodeLength = E_StreamLength(&stm);
	info.byteCode.pShaderBytecode = Sys_Alloc(info.byteCode.BytecodeLength, 1, MH_Transient);
	if (!info.byteCode.pShaderBytecode) {
		E_CloseStream(&stm);
		return;
	}

	if (E_ReadStream(&stm, (void *)info.byteCode.pShaderBytecode, info.byteCode.BytecodeLength) != info.byteCode.BytecodeLength) {
		E_CloseStream(&stm);
		return;
	}

	E_CloseStream(&stm);

	char *name = Rt_TransientStrDup(path);
	name = (char *)strrchr(name, '/');
	*name++ = 0x0;

	char *ext = strrchr(name, '.');
	*ext = 0x0;

	info.hash = Rt_HashString(name);

	Rt_ArrayAdd(&_modules, &info);
}

/* NekoEngine
 *
 * D3D12Shader.cxx
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
