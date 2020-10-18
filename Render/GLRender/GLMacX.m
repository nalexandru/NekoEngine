#define Handle __EngineHandle
#include <System/Log.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Render/Render.h>
#include <Render/Device.h>

#define GLRMOD	L"OpenGLMacX"

#include "GLRender.h"

#undef Handle
#import <Cocoa/Cocoa.h>

static NSOpenGLPixelFormat *_pf;
static NSOpenGLPixelFormatAttribute _pfAttribs[] =
{
	NSOpenGLPFADoubleBuffer,
	NSOpenGLPFAAccelerated,
	NSOpenGLPFADepthSize, 24,
	0, 0, 0, 0,
	0, 0,
	0
};

bool
GL_InitDevice(void)
{
	NSOpenGLContext *ctx;

#if defined(MAC_OS_X_VERSION_10_9) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_9
	_pfAttribs[8] = NSOpenGLPFAOpenGLProfile;
	_pfAttribs[9] = NSOpenGLProfileVersion4_1Core;
#elif defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
	_pfAttribs[8] = NSOpenGLPFAOpenGLProfile;
	_pfAttribs[9] = NSOpenGLProfileVersion3_2Core;
#endif
	
	_pf = [[NSOpenGLPixelFormat alloc] initWithAttributes: _pfAttribs];
	if (!_pf)
		return false;
		
	ctx = [[NSOpenGLContext alloc] initWithFormat: _pf shareContext: nil];
	if (!ctx)
		return false;
	
	Re_Device.loadLock = Sys_AlignedAlloc(sizeof(*Re_Device.loadLock), 64);
	Sys_InitAtomicLock(Re_Device.loadLock);
	
	[ctx setView: [(NSWindow *)E_Screen contentView]];
	[ctx makeCurrentContext];

	Re_Device.glContext = ctx;

	return true;
}

void
GL_SwapBuffers(void)
{
	[(NSOpenGLContext *)Re_Device.glContext flushBuffer];
}

void
GL_SwapInterval(int interval)
{
	[(NSOpenGLContext *)Re_Device.glContext setValues: (long *)&interval forParameter: NSOpenGLCPSwapInterval];
}

void
GL_ScreenResized(void)
{
	[(NSOpenGLContext *)Re_Device.glContext update];
}

void *
GL_InitLoadContext(void)
{
	return [[NSOpenGLContext alloc] initWithFormat: _pf shareContext: (NSOpenGLContext *)Re_Device.glContext];
}

void
GL_MakeCurrent(void *ctx)
{
	[(NSOpenGLContext *)ctx makeCurrentContext];
}

void
GL_TermLoadContext(void *ctx)
{
	[(NSOpenGLContext *)ctx release];
}

void
GL_TermDevice(void)
{
	if (Re_Device.loadLock)
		Sys_AlignedFree(Re_Device.loadLock);

	[NSOpenGLContext clearCurrentContext];
	[(NSOpenGLContext *)Re_Device.glContext release];
}
