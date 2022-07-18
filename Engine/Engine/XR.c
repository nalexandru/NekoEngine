#include <System/Log.h>
#include <System/System.h>
#include <Render/Render.h>
#include <Render/Backend.h>
#include <Engine/Types.h>
#include <Engine/Config.h>
#include <Engine/Engine.h>
#include <Engine/Version.h>
#include <Engine/Application.h>
#include <Engine/BuildConfig.h>
#include <Runtime/Runtime.h>

#define XR_MOD	"OpenXR"

#if ENABLE_OPENXR

#ifdef SYS_PLATFORM_WINDOWS
#	pragma comment(lib, "openxr_loader")
#endif

#include <openxr/openxr.h>

XrSession E_xrSession = 0;
XrInstance E_xrInstance = 0;
XrSystemId E_xrSystemId = 0;
XrActionSet E_masterActionSet = 0;
XrSwapchain E_xrColorSwapchain = 0, E_xrDepthSwapchain = 0;
uint32_t E_xrSwapchainImageCount = 0;
XrSwapchainImageBaseHeader *E_xrColorImages = 0, *E_xrDepthImages = 0;
XrSessionState E_xrState = XR_SESSION_STATE_UNKNOWN;
XrSpace E_xrSceneSpace;
uint32_t E_xrColorId, E_xrDepthId;

bool
E_InitXR(void)
{
	struct NeArray ext = { 0 };
	Rt_InitPtrArray(&ext, 10, MH_Transient);

	Rt_ArrayAddPtr(&ext, XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);

	Re_AppendXrExtensions(&ext);

	XrInstanceCreateInfo ci =
	{
		.type = XR_TYPE_INSTANCE_CREATE_INFO,
		.applicationInfo.apiVersion = XR_CURRENT_API_VERSION,
		.applicationInfo.applicationVersion = XR_MAKE_VERSION(App_applicationInfo.version.major,
			App_applicationInfo.version.minor, App_applicationInfo.version.build),
		.applicationInfo.engineVersion = (uint32_t)XR_MAKE_VERSION(E_VER_MAJOR, E_VER_MINOR, E_VER_BUILD),
		.applicationInfo.apiVersion = XR_CURRENT_API_VERSION,
		.enabledExtensionCount = (uint32_t)ext.count,
		.enabledExtensionNames = (const char **)ext.data,
	//	.enabledApiLayerCount;
	//	.enabledApiLayerNames;
	};

	snprintf(ci.applicationInfo.applicationName, sizeof(ci.applicationInfo.applicationName), "%s", App_applicationInfo.name);
	snprintf(ci.applicationInfo.engineName, sizeof(ci.applicationInfo.engineName), "%s", "NekoEngine");

	if (xrCreateInstance(&ci, &E_xrInstance) != XR_SUCCESS)
		return false;

	XrActionSetCreateInfo asci = { .type = XR_TYPE_ACTION_SET_CREATE_INFO, .priority = 1 };
	snprintf(asci.actionSetName, sizeof(asci.actionSetName), "%s", "NE_MASTER_ACTION_SET");
	snprintf(asci.localizedActionSetName, sizeof(asci.localizedActionSetName), "%s", "NE_MASTER_ACTION_SET");

//	XrResult rc = xrCreateActionSet(E_xrInstance, &asci, &E_masterActionSet);
	//if (xrCreateActionSet(E_xrInstance, &asci, &E_masterActionSet) != XR_SUCCESS)
	//	return false;

	{ // system
		while (true) {
			XrSystemGetInfo si =
			{
				.type = XR_TYPE_SYSTEM_GET_INFO,
				.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY
			};
			XrResult rc = xrGetSystem(E_xrInstance, &si, &E_xrSystemId);

			if (rc == XR_SUCCESS) {
				break;
			} else if (rc == XR_ERROR_FORM_FACTOR_UNAVAILABLE) {
				Sys_MessageBox("Headset not detected", "Headset not detected. Please connect the headset and press OK.", MSG_ICON_WARN);
			} else {
				Sys_LogEntry(XR_MOD, LOG_CRITICAL, "xrGetSystem failed: %d", rc);
				return false;
			}
		}
	}

	return true;
}

bool
E_CreateXrSession(void)
{
	XrSessionCreateInfo sci =
	{
		.type = XR_TYPE_SESSION_CREATE_INFO,
		.next = Re_XrGraphicsBinding(),
		.systemId = E_xrSystemId
	};
	if (xrCreateSession(E_xrInstance, &sci, &E_xrSession) != XR_SUCCESS)
		return false;

	XrSessionActionSetsAttachInfo ai =
	{
		.type = XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO,
		.actionSets = &E_masterActionSet,
		.countActionSets = 1
	};
	if (xrAttachSessionActionSets(E_xrSession, &ai) != XR_SUCCESS)
		return false;

	XrPosef identity =
	{
		.orientation = { .x = 0.f, .y = 0.f, .z = 0.f, .w = 1.f },
		.position = { .x = 0, .y = 0, .z = 0 }
	};
	
	XrReferenceSpaceCreateInfo rsci =
	{
		.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO,
		.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL,
		.poseInReferenceSpace = identity
	};
	if (xrCreateReferenceSpace(E_xrSession, &rsci, &E_xrSceneSpace) != XR_SUCCESS)
		return false;

/*	XrActionSpaceCreateInfo asci =
	{
		.type = XR_TYPE_ACTION_SPACE_CREATE_INFO,
		.action = sw->actions.pose,
	};
	xrStringToPath(E_xrInstance, "/user/hand/left", &asci.subactionPath);
	xrCreateActionSpace(E_xrSession, &asci, &sw->leftHandSpace);

	xrStringToPath(E_xrInstance, "/user/hand/right", &asci.subactionPath);
	xrCreateActionSpace(E_xrSession, &asci, &sw->rightHandSpace);*/

	return true;
}

bool
E_CreateXrSwapchain(void)
{
	XrSwapchainCreateInfo swci =
	{
		.type = XR_TYPE_SWAPCHAIN_CREATE_INFO,
		.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_INPUT_ATTACHMENT_BIT_KHR |
					XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT | XR_SWAPCHAIN_USAGE_TRANSFER_SRC_BIT,
		.sampleCount = 1,
		.faceCount = 1,
		.arraySize = 2,
		.mipCount = 1
	};

	int64_t colorFormat = 0, depthFormat = 0;

	uint32_t count = 0;
	xrEnumerateSwapchainFormats(E_xrSession, 0, &count, NULL);

	int64_t *formats = Sys_Alloc(sizeof(*formats), count, MH_Transient);
	xrEnumerateSwapchainFormats(E_xrSession, count, &count, formats);

	xrEnumerateViewConfigurationViews(E_xrInstance, E_xrSystemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &count, NULL);

	XrViewConfigurationView *cv = Sys_Alloc(sizeof(*cv), count, MH_Transient);
	xrEnumerateViewConfigurationViews(E_xrInstance, E_xrSystemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, count, &count, cv);

	swci.width = cv[0].recommendedImageRectWidth;
	swci.height = cv[0].recommendedImageRectHeight;
	swci.sampleCount = cv[0].recommendedSwapchainSampleCount;

	swci.format = colorFormat;
	if (xrCreateSwapchain(E_xrSession, &swci, &E_xrColorSwapchain) != XR_SUCCESS)
		return false;

	swci.format = depthFormat;
	if (xrCreateSwapchain(E_xrSession, &swci, &E_xrDepthSwapchain) != XR_SUCCESS)
		return false;

	*E_screenWidth = swci.width;
	*E_screenHeight = swci.height;

	xrEnumerateSwapchainImages(E_xrColorSwapchain, 0, &E_xrSwapchainImageCount, NULL);
	E_xrColorImages = Sys_Alloc(sizeof(*E_xrColorImages), E_xrSwapchainImageCount, MH_RenderDriver);
	xrEnumerateSwapchainImages(E_xrColorSwapchain, E_xrSwapchainImageCount, &E_xrSwapchainImageCount, E_xrColorImages);

	xrEnumerateSwapchainImages(E_xrDepthSwapchain, 0, &E_xrSwapchainImageCount, NULL);
	E_xrDepthImages = Sys_Alloc(sizeof(*E_xrDepthImages), E_xrSwapchainImageCount, MH_RenderDriver);
	xrEnumerateSwapchainImages(E_xrDepthSwapchain, E_xrSwapchainImageCount, &E_xrSwapchainImageCount, E_xrDepthImages);

	return true;
}

void
E_XrAcquireImage(void)
{
	XrFrameBeginInfo fbi = { .type = XR_TYPE_FRAME_BEGIN_INFO };
	xrBeginFrame(E_xrSession, &fbi);

	XrSwapchainImageAcquireInfo ai = { .type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	xrAcquireSwapchainImage(E_xrColorSwapchain, &ai, &E_xrColorId);
	xrAcquireSwapchainImage(E_xrDepthSwapchain, &ai, &E_xrDepthId);

	XrSwapchainImageWaitInfo wi = { .type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO, .timeout = 1000 };
	xrWaitSwapchainImage(E_xrColorSwapchain, &wi);
	xrWaitSwapchainImage(E_xrDepthSwapchain, &wi);
}

void
E_XrPresent(void)
{
	if (!E_xrSession)
		return;

	XrSwapchainImageReleaseInfo ri = { .type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	xrReleaseSwapchainImage(E_xrColorSwapchain, &ri);
	xrReleaseSwapchainImage(E_xrDepthSwapchain, &ri);

	XrCompositionLayerProjection pl =
	{
		.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION,
		.space = E_xrSceneSpace,
		.viewCount = 2,
		.views = NULL
	};

	XrFrameEndInfo ei =
	{
		.type = XR_TYPE_FRAME_END_INFO,
		.displayTime = 0,
		.layerCount = 1,
		.layers = NULL,
		.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
	};
	xrEndFrame(E_xrSession, &ei);
}

bool
E_ProcessXrEvents(void)
{
	XrEventDataBuffer ed = { 0 };

	while (E_xrInstance && xrPollEvent(E_xrInstance, &ed) == XR_SUCCESS) {
		switch (ed.type) {
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
			// exit render loop
		break;
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
			XrEventDataSessionStateChanged *evt = (XrEventDataSessionStateChanged *) & ed;

			E_xrState = evt->state;
			switch (evt->state) {
			case XR_SESSION_STATE_READY:
				XrSessionBeginInfo bi =
				{
					.type = XR_TYPE_SESSION_BEGIN_INFO,
					.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO
				};
				xrBeginSession(E_xrSession, &bi);
			break;
			case XR_SESSION_STATE_STOPPING:
				xrEndSession(E_xrSession);
			break;
			case XR_SESSION_STATE_EXITING:
				// exit
			break;
			case XR_SESSION_STATE_LOSS_PENDING:
				// exit
				// restart
			break;
			}
		break;
		}
	}

	return true;
}

void
E_TermXR(void)
{
	if (!E_xrSession)
		return;

	if (E_xrColorSwapchain)
		xrDestroySwapchain(E_xrColorSwapchain);

	if (E_xrDepthSwapchain)
		xrDestroySwapchain(E_xrDepthSwapchain);

	if (E_xrSession)
		xrDestroySession(E_xrSession);
	
	if (E_masterActionSet)
		xrDestroyActionSet(E_masterActionSet);

	if (E_xrInstance)
		xrDestroyInstance(E_xrInstance);

	E_xrSession = 0;
}

#else

void *E_xrInstance = NULL;

bool E_InitXR(void) { return false; }
bool E_CreateXrSession(void) { return false; }
bool E_CreateXrSwapchain(void) { return false; }
void E_XrAcquireImage(void) { }
void E_XrPresent(void) { }
bool E_ProcessXrEvents(void) { return true; }
void E_TermXR(void) { }

#endif
