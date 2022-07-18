#ifndef _NE_ENGINE_RESOURCE_H_
#define _NE_ENGINE_RESOURCE_H_

#include <Engine/IO.h>
#include <Engine/Types.h>
#include <Render/Types.h>

struct NeResourceLoadInfo
{
	struct NeStream stm;
	const char *path;
};

typedef bool (*NeResourceCreateProc)(const char *name, const void *createInfo, void *ptr, NeHandle h);
typedef bool (*NeResourceLoadProc)(struct NeResourceLoadInfo *li, const char *args, void *ptr, NeHandle h);
typedef void (*NeResourceUnloadProc)(void *, NeHandle);

bool E_RegisterResourceType(const char *name, size_t size, NeResourceCreateProc create, NeResourceLoadProc load, NeResourceUnloadProc unload);

NeHandle E_CreateResource(const char *name, const char *type, const void *info);
NeHandle E_LoadResource(const char *path, const char *type);
NeHandle E_AllocateResource(const char *name, const char *type);

void *E_ResourcePtr(NeHandle res);

int32_t	E_ResourceReferences(NeHandle res);
void	E_RetainResource(NeHandle res);
void	E_ReleaseResource(NeHandle res);

static inline uint16_t	E_ResHandleToGPU(NeHandle h) { return (uint16_t)(h & (uint64_t)0x000000000000FFFF); }
NeHandle				E_GPUHandleToRes(uint16_t id, const char *type);

bool	E_UpdateResource(NeHandle res, const void *data);

void	E_UnloadResource(NeHandle res);

void	E_PurgeResources(void);

bool	E_InitResourceSystem(void);
void	E_TermResourceSystem(void);

#endif /* _NE_ENGINE_RESOURCE_H_ */
