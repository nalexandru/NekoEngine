#include <System/Log.h>
#include <System/Memory.h>

#include "VulkanDriver.h"

static void *_Alloc(void *ud, size_t size, size_t align, VkSystemAllocationScope scope);
static void *_Realloc(void *ud, void *mem, size_t size, size_t align, VkSystemAllocationScope scope);
static void _Free(void *ud, void *mem);

//static void _InternalAlloc(void *ud, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope);
//static void _InternalFree(void *ud, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope);

struct VkAllocationCallbacks _transientAllocCB =
{
	.pfnAllocation = _Alloc,
	.pfnReallocation = _Realloc,
	.pfnFree = _Free
};
VkAllocationCallbacks *Vkd_allocCb = NULL, *Vkd_transientAllocCb = &_transientAllocCB;

static void *
_Alloc(void *ud, size_t size, size_t align, VkSystemAllocationScope scope)
{
	return Sys_Alloc(size, 1, MH_Frame);
}

static void *
_Realloc(void *ud, void *mem, size_t size, size_t align, VkSystemAllocationScope scope)
{
	return NULL; // transient memory cannot be realloc'd
}

static void
_Free(void *ud, void *mem)
{
	(void)ud; (void)mem;
}
