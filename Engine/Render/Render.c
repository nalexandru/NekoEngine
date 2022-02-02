#include <stdlib.h>
#include <string.h>

#include <System/Log.h>
#include <System/System.h>
#include <System/Thread.h>
#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Engine/Config.h>
#include <Engine/Engine.h>
#include <Engine/Resource.h>
#include <Render/Model.h>
#include <Render/Render.h>
#include <Render/Graph/Graph.h>
#include <Render/Driver/Driver.h>

#define RE_MOD "Render"
#define CHK_FAIL(x, y) if (!x) { Sys_LogEntry(RE_MOD, LOG_CRITICAL, y); return false; }

#ifdef SYS_PLATFORM_APPLE
#	define DEFAULT_RENDER_DRIVER	"Metal"
#else
#	define DEFAULT_RENDER_DRIVER	"Vulkan"
#endif

struct NeRenderDevice *Re_device;
struct NeRenderDeviceInfo Re_deviceInfo = { 0 };
struct NeRenderDeviceProcs Re_deviceProcs = { 0 };
struct NeRenderContextProcs Re_contextProcs = { 0 };

struct NeSurface *Re_surface = NULL;
struct NeSwapchain *Re_swapchain = NULL;
THREAD_LOCAL struct NeRenderContext *Re_context = NULL;
const struct NeRenderDriver *Re_driver = NULL;
struct NeRenderContext **Re_contexts = NULL;

#ifdef USE_STATIC_RENDER_DRIVER
const struct NeRenderDriver *Re_LoadStaticDriver(void);
#endif

bool
Re_InitRender(void)
{
	uint32_t devCount = 0;
	struct NeRenderDeviceInfo *info = NULL, *selected = NULL;

	if (E_GetCVarBln("Render_WaitForDebugger", false)->bln)
		Sys_MessageBox("NekoEngine", "Attach the graphics debugger now", MSG_ICON_INFO);

#ifndef USE_STATIC_RENDER_DRIVER
	ReLoadDriverProc loadDriver = NULL;
	void *module = Sys_LoadLibrary(NULL);
	CHK_FAIL(module, "Failed to load executable");

	const char *drvName = E_GetCVarStr("Render_Driver", DEFAULT_RENDER_DRIVER)->str;

	char name[256];
	snprintf(name, sizeof(name), "Re_Load%sDriver", drvName);

	loadDriver = Sys_GetProcAddress(module, name);
	CHK_FAIL(loadDriver, "The specified driver does not exist");

	Re_driver = loadDriver();
#else
	Re_driver = Re_LoadStaticDriver();
#endif

	CHK_FAIL(Re_driver, "Failed to load driver");
	CHK_FAIL((Re_driver->identifier == NE_RENDER_DRIVER_ID), "The library is not a valid driver");
	CHK_FAIL((Re_driver->apiVersion == NE_RENDER_DRIVER_API), "Driver version mismatch");
	CHK_FAIL(Re_driver->Init(), "Failed to initialize driver");
	CHK_FAIL(Re_driver->EnumerateDevices(&devCount, NULL), "Failed to enumerate devices");

	info = Sys_Alloc(sizeof(*info), devCount, MH_Transient);
	CHK_FAIL(info, "Failed to enumerate devices");
	CHK_FAIL(Re_driver->EnumerateDevices(&devCount, info), "Failed to enumerate devices");

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

	Re_device = Re_driver->CreateDevice(&Re_deviceInfo, &Re_deviceProcs, &Re_contextProcs);
	CHK_FAIL(Re_device, "Failed to create device");

	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "GPU: %s (%hX:%hX)", Re_deviceInfo.deviceName,
											Re_deviceInfo.hardwareInfo.vendorId, Re_deviceInfo.hardwareInfo.deviceId);
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "\tAPI: %s", Re_driver->driverName);
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "\tMemory: %llu MB", Re_deviceInfo.localMemorySize / 1024 / 1024);
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "\tRay Tracing: %s", Re_deviceInfo.features.rayTracing ? "yes" : "no");
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "\tMesh Shading: %s", Re_deviceInfo.features.meshShading ? "yes" : "no");
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "\tUnified Memory: %s", Re_deviceInfo.features.unifiedMemory ? "yes" : "no");
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, "\tCoherent Memory: %s", Re_deviceInfo.features.coherentMemory ? "yes" : "no");

	Re_surface = Re_CreateSurface(E_screen);
	CHK_FAIL(Re_surface, "Failed to create surface");

	Re_swapchain = Re_CreateSwapchain(Re_surface, E_GetCVarBln("Render_VerticalSync", false)->bln);
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

	Re_driver->DestroyDevice(Re_device);
	Re_driver->Term();
}
