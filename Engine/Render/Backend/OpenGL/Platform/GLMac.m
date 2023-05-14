#include "../GLBackend.h"

#import <Cocoa/Cocoa.h>
#import <IOKit/IOKitLib.h>
#include <mach-o/dyld.h>

#include <System/Log.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>

#define GLMAC_MOD	"GLMac"

static NSOpenGLContext *f_ctx;
static NSOpenGLPixelFormat *f_pf;
static NSOpenGLPixelFormatAttribute f_pfAttribs[] =
{
	NSOpenGLPFADoubleBuffer,
	NSOpenGLPFAAccelerated,
	NSOpenGLPFANoRecovery,
	NSOpenGLPFAWindow,
	NSOpenGLPFAMinimumPolicy,
	NSOpenGLPFAColorSize, 24,
	NSOpenGLPFAAlphaSize, 8,
	NSOpenGLPFADepthSize, 24,
	NSOpenGLPFAStencilSize, 8,
	NSOpenGLPFAAccumSize, 0,
	0, 0, 0, 0, 0,
	0, 0,
	0
};

bool
GLBk_InitContext(void)
{
	NSOpenGLPixelFormatAttribute *aptr = &f_pfAttribs[15];
	
#if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
	*aptr++ = NSOpenGLPFAOpenGLProfile;
	*aptr++ = NSOpenGLProfileVersionLegacy;
#endif
	
	if (CVAR_BOOL("Render_Multisampling")) {
		*aptr++ = NSOpenGLPFAMultisample;
		*aptr++ = NSOpenGLPFASampleBuffers;
		*aptr++ = 1;
		*aptr++ = NSOpenGLPFASamples;
		*aptr++ = E_GetCVarI32("Render_Samples", 4)->i32;
	}
	
	f_pf = [[NSOpenGLPixelFormat alloc] initWithAttributes: f_pfAttribs];
	if (!f_pf) 
		return false;
	
	f_ctx = [[NSOpenGLContext alloc] initWithFormat: f_pf shareContext: nil];
	if (!f_ctx)
		return false;
	
	[f_ctx setView: (NSView *)E_screen];
	[f_ctx makeCurrentContext];
	
	gladLoadGL();
	
	return true;
}

void *
GLBk_CreateShareContext(void)
{
	return [[NSOpenGLContext alloc] initWithFormat: f_pf shareContext: f_ctx];
}

void
GLBk_DestroyShareContext(void *ctx)
{
	[(NSOpenGLContext *)ctx release];
}

void
GLBk_MakeCurrent(void *ctx)
{
	[(NSOpenGLContext *)ctx makeCurrentContext];
}

void
GLBk_EnableVerticalSync(bool enable)
{
	GLint interval = enable;
	[f_ctx setValues: &interval forParameter: NSOpenGLCPSwapInterval];
}

void
GLBk_SwapBuffers(void)
{
	[f_ctx flushBuffer];
}

void
GLBk_HardwareInfo(struct NeRenderDeviceInfo *info)
{
	long size = 0;
	
	io_registry_entry_t port = CGDisplayIOServicePort(kCGDirectMainDisplay);
	
	CFDataRef ref = IORegistryEntrySearchCFProperty(port, kIOServicePlane,
													CFSTR("vendor-id"), kCFAllocatorDefault,
													kIORegistryIterateRecursively | kIORegistryIterateParents);
	if (ref) {
		info->hardwareInfo.vendorId = *((uint32_t*)CFDataGetBytePtr(ref));
		CFRelease(ref);
	}
	
	ref = IORegistryEntrySearchCFProperty(port, kIOServicePlane,
										  CFSTR("device-id"), kCFAllocatorDefault,
										  kIORegistryIterateRecursively | kIORegistryIterateParents);
	if (ref) {
		info->hardwareInfo.deviceId = *((uint32_t*)CFDataGetBytePtr(ref));
		CFRelease(ref);
	}
	
	CFTypeRef typeCode = IORegistryEntrySearchCFProperty(port, kIOServicePlane, CFSTR(kIOFBMemorySizeKey),
														 kCFAllocatorDefault,kIORegistryIterateRecursively | kIORegistryIterateParents);
	
	if (typeCode && CFGetTypeID(typeCode) == CFNumberGetTypeID()) {
		CFNumberGetValue(typeCode, kCFNumberSInt32Type, &size);
		CFRelease(typeCode);
	}
	
	info->localMemorySize = size;
}

void
GLBk_TermContext(void)
{
	[NSOpenGLContext clearCurrentContext];
	[f_ctx release];
}
