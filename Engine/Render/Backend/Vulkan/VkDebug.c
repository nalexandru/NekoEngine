#include <time.h>

#include <System/Log.h>
#include <System/Thread.h>

#include "VulkanBackend.h"

#if (defined(SYS_PLATFORM_WINDOWS) || defined(SYS_PLATFORM_LINUX)) && (ENABLE_AFTERMATH == 1)
#	include <System/System.h>
#	include <Engine/Version.h>
#	include <Engine/Application.h>

#	include <GFSDK_Aftermath.h>
#	include <GFSDK_Aftermath_GpuCrashDump.h>
#	include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>

#	define VKD_AFTERMATH

static void _InitAftermath(void);
static void _TermAftermath(void);

#endif

static VkDebugUtilsMessengerEXT _msg;

static VkBool32 VKAPI_CALL _debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
											const VkDebugUtilsMessengerCallbackDataEXT *, void *);

static inline const char *
_objectType(VkObjectType type)
{
	switch (type) {
	case VK_OBJECT_TYPE_INSTANCE: return "Instance";
	case VK_OBJECT_TYPE_PHYSICAL_DEVICE: return "Physical Device";
	case VK_OBJECT_TYPE_DEVICE: return "Device";
	case VK_OBJECT_TYPE_QUEUE: return "Queue";
	case VK_OBJECT_TYPE_SEMAPHORE: return "Semaphore";
	case VK_OBJECT_TYPE_COMMAND_BUFFER: return "Command Buffer";
	case VK_OBJECT_TYPE_FENCE: return "Fence";
	case VK_OBJECT_TYPE_DEVICE_MEMORY: return "Device Memory";
	case VK_OBJECT_TYPE_BUFFER: return "Buffer";
	case VK_OBJECT_TYPE_IMAGE: return "Image";
	case VK_OBJECT_TYPE_EVENT: return "Event";
	case VK_OBJECT_TYPE_QUERY_POOL: return "Query Pool";
	case VK_OBJECT_TYPE_BUFFER_VIEW: return "Buffer View";
	case VK_OBJECT_TYPE_IMAGE_VIEW: return "Image View";
	case VK_OBJECT_TYPE_SHADER_MODULE: return "Shader Module";
	case VK_OBJECT_TYPE_PIPELINE_CACHE: return "Pipeline Cache";
	case VK_OBJECT_TYPE_PIPELINE_LAYOUT: return "Pipeline Layout";
	case VK_OBJECT_TYPE_RENDER_PASS: return "Render Pass";
	case VK_OBJECT_TYPE_PIPELINE: return "Pipeline";
	case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: return "Descriptor Set Layout";
	case VK_OBJECT_TYPE_SAMPLER: return "Sampler";
	case VK_OBJECT_TYPE_DESCRIPTOR_POOL: return "Descriptor Pool";
	case VK_OBJECT_TYPE_DESCRIPTOR_SET: return "Descriptor Set";
	case VK_OBJECT_TYPE_FRAMEBUFFER: return "Framebuffer";
	case VK_OBJECT_TYPE_COMMAND_POOL: return "Command Pool";
	case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION: return "Sampler YCbCr Conversion";
	case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE: return "Descriptor Update Template";
	case VK_OBJECT_TYPE_SURFACE_KHR: return "Surface";
	case VK_OBJECT_TYPE_SWAPCHAIN_KHR: return "Swapchain";
	case VK_OBJECT_TYPE_DISPLAY_KHR: return "Display";
	case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: return "Display Mode";
	case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT: return "Debug Report Callback";
	case VK_OBJECT_TYPE_VIDEO_SESSION_KHR: return "Video Session";
	case VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR: return "Video Session Parameters";
#ifdef VK_OBJECT_TYPE_CU_MODULE_NVX
	case VK_OBJECT_TYPE_CU_MODULE_NVX: return "Cuda Module";
	case VK_OBJECT_TYPE_CU_FUNCTION_NVX: return "Cuda Function";
#endif
	case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT: return "Debug Utils Messenger";
	case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR: return "Acceleration Structure";
	case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT: return "Validation Cache";
	case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV: return "Acceleration Structure NV";
	case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL: return "Performance Configuration";
	case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR: return "Deferred Operation";
	case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV: return "Indirect Commands Layout";
	case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT: return "Private Data Slot";
	case VK_OBJECT_TYPE_UNKNOWN: return "Unknown";
	}

	return "Unknown";
}

bool
Vkd_InitDebug(void)
{		
	VkDebugUtilsMessengerCreateInfoEXT ci =
	{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
						VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
							VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.pfnUserCallback = _debugCallback
	};
	if (vkCreateDebugUtilsMessengerEXT(Vkd_inst, &ci, Vkd_allocCb, &_msg) != VK_SUCCESS)
		return false;

#ifdef VKD_AFTERMATH
	_InitAftermath();
#endif
	
	return true;
}

bool
Vkd_SetObjectName(VkDevice dev, void *handle, VkObjectType type, const char *name)
{
	if (!vkSetDebugUtilsObjectNameEXT)
		return true;

	VkDebugUtilsObjectNameInfoEXT info =
	{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
		.objectHandle = (uint64_t)handle,
		.objectType = type,
		.pObjectName = name
	};
	return vkSetDebugUtilsObjectNameEXT(dev, &info) == VK_SUCCESS;
}

void
Vkd_TermDebug(void)
{
	vkDestroyDebugUtilsMessengerEXT(Vkd_inst, _msg, Vkd_allocCb);
	
#ifdef VKD_AFTERMATH
	_TermAftermath();
#endif
}

static VkBool32 VKAPI_CALL _debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT flags,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT *data,
	void *userData)
{
	if (!data->messageIdNumber)
		return VK_FALSE;

	const int severity = (flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? LOG_CRITICAL :
		(flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ? LOG_WARNING :
			(flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT ? LOG_INFORMATION : LOG_DEBUG));

  /*  uint32_t                                     queueLabelCount;
    const VkDebugUtilsLabelEXT*                  pQueueLabels;
    uint32_t                                     cmdBufLabelCount;
    const VkDebugUtilsLabelEXT*                  pCmdBufLabels;*/

	Sys_LogEntry(VKDRV_MOD, severity, "[%d]: %s", data->messageIdNumber, data->pMessage);

	for (uint32_t i = 0; i < data->objectCount; ++i)
		Sys_LogEntry(VKDRV_MOD, severity, "\tObject [%llu][%s][%s]", data->pObjects[i].objectHandle,
										_objectType(data->pObjects[i].objectType), data->pObjects[i].pObjectName);

	return VK_FALSE;
}

#ifdef VKD_AFTERMATH

static char _aftermathDumpRoot[256];
static NeMutex _aftermathMutex;

static void *_aftermathModule;
static PFN_GFSDK_Aftermath_EnableGpuCrashDumps _Aftermath_EnableGpuCrashDumps;
static PFN_GFSDK_Aftermath_DisableGpuCrashDumps _Aftermath_DisableGpuCrashDumps;
static PFN_GFSDK_Aftermath_GpuCrashDump_CreateDecoder _Aftermath_GpuCrashDump_CreateDecoder;
static PFN_GFSDK_Aftermath_GpuCrashDump_GenerateJSON _Aftermath_GpuCrashDump_GenerateJSON;
static PFN_GFSDK_Aftermath_GpuCrashDump_GetJSON _Aftermath_GpuCrashDump_GetJSON;
static PFN_GFSDK_Aftermath_GpuCrashDump_DestroyDecoder _Aftermath_GpuCrashDump_DestroyDecoder;
static PFN_GFSDK_Aftermath_GetShaderDebugInfoIdentifier _Aftermath_GetShaderDebugInfoIdentifier;

static void _GpuCrashDumpCb(const void *pGpuCrashDump, const uint32_t gpuCrashDumpSize, void *pUserData);
static void _ShaderDebugInfoCb(const void *pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void *pUserData);
static void _GpuCrashDumpDescriptionCb(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addValue, void *pUserData);

static void _ShaderDebugInfoLookupCb(const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo, void* pUserData);
static void _ShaderLookupCb(const GFSDK_Aftermath_ShaderHash* pShaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData);
static void _ShaderInstructionsLookupCb(const GFSDK_Aftermath_ShaderInstructionsHash* pShaderInstructionsHash, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData);
static void _ShaderSourceDebugInfoLookupCb(const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData);

static void
_InitAftermath(void)
{
	_aftermathModule = Sys_LoadLibrary("GFSDK_Aftermath_Lib.x64");
	if (!_aftermathModule)
		return;

#define LOAD_PROC(x) \
	x = Sys_GetProcAddress(_aftermathModule, "GFSDK" #x);	\
	if (!x) goto unload

	LOAD_PROC(_Aftermath_EnableGpuCrashDumps);
	LOAD_PROC(_Aftermath_DisableGpuCrashDumps);
	LOAD_PROC(_Aftermath_GpuCrashDump_CreateDecoder);
	LOAD_PROC(_Aftermath_GpuCrashDump_GenerateJSON);
	LOAD_PROC(_Aftermath_GpuCrashDump_GetJSON);
	LOAD_PROC(_Aftermath_GpuCrashDump_DestroyDecoder);
	LOAD_PROC(_Aftermath_GetShaderDebugInfoIdentifier);

#undef LOAD_PROC

	Sys_InitMutex(&_aftermathMutex);

	GFSDK_Aftermath_Result rc = _Aftermath_EnableGpuCrashDumps(
		GFSDK_Aftermath_Version_API,
		GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan,
		GFSDK_Aftermath_GpuCrashDumpFeatureFlags_DeferDebugInfoCallbacks,
		_GpuCrashDumpCb,
		_ShaderDebugInfoCb,
		_GpuCrashDumpDescriptionCb,
		NULL
	);

	if (rc != GFSDK_Aftermath_Result_Success)
		Sys_LogEntry(VKDRV_MOD, LOG_CRITICAL, "Failed to initialize NVIDIA Aftermath: %d", rc);

	return;

unload:
	Sys_UnloadLibrary(_aftermathModule);
}

static void
_TermAftermath(void)
{
	if (!_aftermathModule)
		return;

	_Aftermath_DisableGpuCrashDumps();

	Sys_TermMutex(_aftermathMutex);
	Sys_UnloadLibrary(_aftermathModule);
}

static void
_GpuCrashDumpCb(const void *pGpuCrashDump, const uint32_t gpuCrashDumpSize, void *pUserData)
{
	Sys_LockMutex(_aftermathMutex);

	GFSDK_Aftermath_GpuCrashDump_Decoder decoder = { 0 };
	GFSDK_Aftermath_Result rc = _Aftermath_GpuCrashDump_CreateDecoder(
		GFSDK_Aftermath_Version_API,
		pGpuCrashDump,
		gpuCrashDumpSize,
		&decoder
	);

	char appData[256];
	Sys_DirectoryPath(SD_APP_DATA, appData, sizeof(appData));

	time_t t = time(0);
	struct tm *tm = localtime(&t);

	Sys_ZeroMemory(_aftermathDumpRoot, sizeof(_aftermathDumpRoot));
	snprintf(_aftermathDumpRoot, sizeof(_aftermathDumpRoot), "%s/GpuDump/Vulkan/%04d-%02d-%02d_%02d:%02d:%02d", appData,
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);
	Sys_CreateDirectory(_aftermathDumpRoot);

	char path[256];
	snprintf(path, sizeof(path), "%s/crash.nv-gpudmp", _aftermathDumpRoot);

	FILE *fp = fopen(path, "wb");
	if (fp) {
		fwrite(pGpuCrashDump, gpuCrashDumpSize, 1, fp);
		fclose(fp);
	}

	uint32_t jsonSize = 0;
	rc = _Aftermath_GpuCrashDump_GenerateJSON(
		decoder,
		GFSDK_Aftermath_GpuCrashDumpDecoderFlags_ALL_INFO,
		GFSDK_Aftermath_GpuCrashDumpFormatterFlags_NONE,
		_ShaderDebugInfoLookupCb,
		_ShaderLookupCb,
		_ShaderInstructionsLookupCb,
		_ShaderSourceDebugInfoLookupCb,
		NULL,
		&jsonSize
	);

	char *json = Sys_Alloc(sizeof(*json), jsonSize, MH_System);
	rc = _Aftermath_GpuCrashDump_GetJSON(decoder, jsonSize, json);

	strncat(path, ".json", sizeof(path) - strlen(path));
	fp = fopen(path, "wb");
	if (fp) {
		fwrite(json, sizeof(*json), jsonSize, fp);
		fclose(fp);
	}

	Sys_Free(json);
	_Aftermath_GpuCrashDump_DestroyDecoder(decoder);

	Sys_UnlockMutex(_aftermathMutex);
}

static void
_ShaderDebugInfoCb(const void *pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void *pUserData)
{
	Sys_LockMutex(_aftermathMutex);

	GFSDK_Aftermath_ShaderDebugInfoIdentifier sdii;
	GFSDK_Aftermath_Result rc = _Aftermath_GetShaderDebugInfoIdentifier(
		GFSDK_Aftermath_Version_API,
		pShaderDebugInfo,
		shaderDebugInfoSize,
		&sdii
	);

	char path[256];
#ifdef _WIN32
	snprintf(path, sizeof(path), "%s/shader-%llu%llu.nvdbg", _aftermathDumpRoot, sdii.id[0], sdii.id[1]);
#else
	snprintf(path, sizeof(path), "%s/shader-%lu%lu.nvdbg", _aftermathDumpRoot, sdii.id[0], sdii.id[1]);
#endif

	FILE *fp = fopen(path, "wb");
	if (fp) {
		fwrite(pShaderDebugInfo, shaderDebugInfoSize, 1, fp);
		fclose(fp);
	}

	Sys_UnlockMutex(_aftermathMutex);
}

static void
_GpuCrashDumpDescriptionCb(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addValue, void *pUserData)
{
	addValue(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName, App_applicationInfo.name);

	char verStr[16];
	snprintf(verStr, sizeof(verStr), "%d.%d.%d.%d", App_applicationInfo.version.major,
		App_applicationInfo.version.minor, App_applicationInfo.version.build, App_applicationInfo.version.revision);
	addValue(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationVersion, verStr);

	addValue(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined, E_VER_STR);
	addValue(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined + 1, Sys_OperatingSystem());
	addValue(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined + 2, Sys_OperatingSystemVersionString());
	addValue(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined + 3, Sys_Machine());
}

static void
_ShaderDebugInfoLookupCb(const GFSDK_Aftermath_ShaderDebugInfoIdentifier *pIdentifier,
	PFN_GFSDK_Aftermath_SetData setShaderDebugInfo, void *pUserData)
{
	// TODO: https://github.com/NVIDIA/nsight-aftermath-samples/blob/987837f656b69cb0252c954f62a468c0aab61c6e/D3D12HelloNsightAftermath/NsightAftermathGpuCrashTracker.cpp#L297
}

static void
_ShaderLookupCb(const GFSDK_Aftermath_ShaderHash *pShaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary, void *pUserData)
{

}

static void
_ShaderInstructionsLookupCb(const GFSDK_Aftermath_ShaderInstructionsHash *pShaderInstructionsHash,
	PFN_GFSDK_Aftermath_SetData setShaderBinary, void *pUserData)
{

}

static void
_ShaderSourceDebugInfoLookupCb(const GFSDK_Aftermath_ShaderDebugName *pShaderDebugName,
	PFN_GFSDK_Aftermath_SetData setShaderBinary, void *pUserData)
{

}

#endif /* (defined(SYS_PLATFORM_WINDOWS) || defined(SYS_PLATFORM_LINUX)) && defined(ENABLE_AFTERMATH) */
