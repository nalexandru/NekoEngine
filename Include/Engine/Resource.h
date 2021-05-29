#ifndef _NE_ENGINE_RESOURCE_H_
#define _NE_ENGINE_RESOURCE_H_

#include <Engine/IO.h>
#include <Engine/Types.h>

struct ResourceLoadInfo
{
	struct Stream stm;
	const char *path;
};

typedef bool(*ResourceCreateProc)(const char *name, const void *createInfo, void *ptr, Handle h);
typedef bool(*ResourceLoadProc)(struct ResourceLoadInfo *li, const char *args, void *ptr, Handle h);
typedef void (*ResourceUnloadProc)(void *, Handle);

bool E_RegisterResourceType(const char *name, size_t size, ResourceCreateProc create, ResourceLoadProc load, ResourceUnloadProc unload);

Handle E_CreateResource(const char *name, const char *type, const void *info);
Handle E_LoadResource(const char *path, const char *type);

void *E_ResourcePtr(Handle res);

int32_t	E_ResourceReferences(Handle res);
void	E_RetainResource(Handle res);
void	E_ReleaseResource(Handle res);

static inline uint16_t	E_ResHandleToGPU(Handle h) { return (uint16_t)(h & (uint64_t)0x000000000000FFFF); }
Handle					E_GPUHandleToRes(uint16_t id, const char *type);

void	E_UnloadResource(Handle res);

void	E_PurgeResources(void);

bool	E_InitResourceSystem(void);
void	E_TermResourceSystem(void);

#endif /* _NE_ENGINE_RESOURCE_H_ */
