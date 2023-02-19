#include <stdlib.h>
#include <string.h>

#include <System/Log.h>
#include <System/System.h>
#include <System/Thread.h>
#include <Engine/XR.h>
#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Engine/Config.h>
#include <Engine/Engine.h>
#include <Engine/Resource.h>
#include <Render/Core.h>
#include <Render/Model.h>
#include <Render/Render.h>
#include <Render/Graph/Graph.h>
#include <Render/Backend.h>

// This is not part of the exposed IO API
const char *E_RealPath(const char *path);

#define RE_MOD "Render"
#define CHK_FAIL(x, y) if (!x) { Sys_LogEntry(RE_MOD, LOG_CRITICAL, y); return false; }

struct NeRenderDevice *Re_device;
struct NeRenderDeviceInfo Re_deviceInfo = { 0 };

struct NeSurface *Re_surface = NULL;
struct NeSwapchain *Re_swapchain = NULL;
THREAD_LOCAL struct NeRenderContext *Re_context = NULL;
struct NeRenderContext **Re_contexts = NULL;

bool
Re_InitRender(void)
{
	uint32_t devCount = 0;
	struct NeRenderDeviceInfo *info = NULL, *selected = NULL;

	if (E_GetCVarBln("Render_WaitForDebugger", false)->bln)
		Sys_MessageBox("NekoEngine", "Attach the graphics debugger now", MSG_ICON_INFO);

#ifndef SYS_PLATFORM_APPLE
	if (CVAR_BOOL("Engine_EnableXR"))
		CHK_FAIL(E_InitXR(), "Failed to create OpenXR instance");
#endif

	CHK_FAIL(Re_InitBackend(), "Failed to initialize backend");
	CHK_FAIL(Re_EnumerateDevices(&devCount, NULL), "Failed to enumerate devices");

	info = Sys_Alloc(sizeof(*info), devCount, MH_Transient);
	CHK_FAIL(info, "Failed to enumerate devices");
	CHK_FAIL(Re_EnumerateDevices(&devCount, info), "Failed to enumerate devices");

	if (E_GetCVarI32("Render_Adapter", -1)->i32 != -1) {
		int32_t i = E_GetCVarI32("Render_Adapter", -1)->i32;
		CHK_FAIL(((uint32_t)i < devCount), "Invalid adapter specified");
		selected = &info[i];
	} else {
		uint64_t vramSize = 0;
		bool haveRt = false;
		for (uint32_t i = 0; i < devCount; ++i) {
			if (!info[i].features.canPresent)
				continue;
	
			if (info[i].features.rayTracing) {
				if (!haveRt || (haveRt && vramSize < info[i].localMemorySize))
					goto updateSelection;
			} else {
				if (vramSize < info[i].localMemorySize)
					goto updateSelection;
			}
	
			continue;
	
		updateSelection:
			selected = &info[i];
			vramSize = info[i].localMemorySize;
			haveRt = info[i].features.rayTracing;
		}
	}

	CHK_FAIL(selected, "No suitable device found");

	memcpy(&Re_deviceInfo, selected, sizeof(Re_deviceInfo));

	Re_device = Re_CreateDevice(&Re_deviceInfo);
	CHK_FAIL(Re_device, "Failed to create device");

	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "GPU: %s (%hX:%hX)", Re_deviceInfo.deviceName,
											Re_deviceInfo.hardwareInfo.vendorId, Re_deviceInfo.hardwareInfo.deviceId);
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "\tAPI: %s", Re_backendName);
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "\tMemory: %llu MB", Re_deviceInfo.localMemorySize / 1024 / 1024);
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "\tRay Tracing: %s", Re_deviceInfo.features.rayTracing ? "yes" : "no");
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "\tMesh Shading: %s", Re_deviceInfo.features.meshShading ? "yes" : "no");
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "\tUnified Memory: %s", Re_deviceInfo.features.unifiedMemory ? "yes" : "no");
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "\tCoherent Memory: %s", Re_deviceInfo.features.coherentMemory ? "yes" : "no");

	Re_surface = Re_CreateSurface(E_screen);
	CHK_FAIL(Re_surface, "Failed to create surface");

	Re_swapchain = Re_CreateSwapchain(Re_surface, E_GetCVarBln("Render_VerticalSync", true)->bln);
	CHK_FAIL(Re_swapchain, "Failed to create swapchain");

	CHK_FAIL(Re_LoadShaders(), "Failed to load shaders");
	CHK_FAIL(Re_InitPipelines(), "Failed to create pipelines");

	Re_contexts = Sys_Alloc((uint64_t)E_JobWorkerThreads() + 1, sizeof(*Re_contexts), MH_Render);
	CHK_FAIL(Re_contexts, "Failed to allocate contexts");

	for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i)
		Re_contexts[i] = Re_CreateContext();
	Re_context = Re_contexts[E_JobWorkerThreads()];

	CHK_FAIL(Re_InitResourceDestructor(), "Failed to initialize resource destructor");
	CHK_FAIL(Re_InitTransientHeap(E_GetCVarU64("Render_TransientHeapSize", 128 * 1024 * 1024)->u64), "Failed to initialize transient heap");

	CHK_FAIL(Re_InitBufferSystem(), "Failed to initialize buffer system");
	CHK_FAIL(Re_InitTextureSystem(), "Failed to initialize texture system");
	CHK_FAIL(Re_InitMaterialSystem(), "Failed to initialize material system");

	CHK_FAIL(E_RegisterResourceType(RES_MODEL, sizeof(struct NeModel), (NeResourceCreateProc)Re_CreateModelResource,
							(NeResourceLoadProc)Re_LoadModelResource, (NeResourceUnloadProc)Re_UnloadModelResource),
			"Failed to register model resource");

	return true;
}

NeDirectIOHandle
Re_OpenFile(const char *path)
{
	const char *realPath = E_RealPath(path);
	if (!realPath)
		return NULL;	// This can happen if the file is inside an archive
	return Re_BkOpenFile(path);
}

void
Re_CloseFile(NeDirectIOHandle handle)
{
	Re_BkCloseFile(handle);
}

void
Re_TermRender(void)
{
	Re_WaitIdle();

	Re_DestroyGraph(Re_activeGraph);

	Re_TermMaterialSystem();

	Re_TermTransientHeap();
	Re_TermResourceDestructor();

	Re_TermTextureSystem();
	Re_TermBufferSystem();

	for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i)
		Re_DestroyContext(Re_contexts[i]);
	Sys_Free(Re_contexts);

	Re_TermPipelines();

	Re_UnloadShaders();

	Re_DestroySwapchain(Re_swapchain);
	Re_DestroySurface(Re_surface);

	Re_DestroyDevice(Re_device);
	Re_TermBackend();

	if (E_xrInstance)
		E_TermXR();
}

/* NekoEngine
 *
 * Render.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
