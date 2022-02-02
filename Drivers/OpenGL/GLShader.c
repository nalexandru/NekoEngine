#include <Engine/IO.h>
#include <System/Log.h>
#include <Runtime/Runtime.h>

#include "OpenGLDriver.h"

struct ShaderModuleInfo
{
	uint64_t hash;
//	VkShaderModule module;
};

static struct NeArray _modules;

//static VkDevice _dev;
//static void _load(const char *path);
//static int32_t _sort(const struct ShaderModuleInfo *a, const struct ShaderModuleInfo *b);
//static int32_t _compare(const struct ShaderModuleInfo *item, const uint64_t *hash);

void *
GL_ShaderModule(struct NeRenderDevice *dev, const char *name)
{
/*	uint64_t hash = Rt_HashString(name);

	struct ShaderModuleInfo *info = Rt_ArrayBSearch(&_modules, &hash, (RtCmpFunc)_compare);
	if (!info)
		return NULL;

	return info->module;*/
	return NULL;
}

bool
GL_LoadShaders(void)
{
/*	if (!E_MountMemory("Shaders.zip", Shaders_zip, sizeof(Shaders_zip), "/"))
		return false;

	if (!Rt_InitArray(&_modules, 10, sizeof(struct ShaderModuleInfo), MH_RenderDriver))
		return false;

	_dev = dev;

	E_ProcessFiles("/Shaders/Vulkan", "spv", true, _load);
	Rt_ArraySort(&_modules, (RtSortFunc)&_sort);*/

	return true;
}

void
GL_UnloadShaders(void)
{
/*	struct ShaderModuleInfo *info;
	Rt_ArrayForEach(info, &_modules)
		vkDestroyShaderModule(dev, info->module, Vkd_allocCb);
	Rt_TermArray(&_modules);
	E_Unmount("Shaders.zip");*/
}

/*void
_load(const char *path)
{
	struct ShaderModuleInfo info;

	struct Stream stm;
	if (!E_FileStream(path, IO_READ, &stm))
		return;

	int64_t size = E_StreamLength(&stm);
	void *data = Sys_Alloc(size, 1, MH_Transient);
	if (!data) {
		E_CloseStream(&stm);
		return;
	}

	if (E_ReadStream(&stm, data, size) != size) {
		E_CloseStream(&stm);
		return;
	}

	E_CloseStream(&stm);

	char *name = Sys_Alloc(sizeof(*name), strlen(path), MH_Transient);
	name = strrchr(path, '/');
	*name++ = 0x0;

	char *ext = strrchr(name, '.');
	*ext = 0x0;

	info.hash = Rt_HashString(name);

	VkShaderModuleCreateInfo ci =
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = size,
		.pCode = data
	};
	if (vkCreateShaderModule(_dev, &ci, Vkd_allocCb, &info.module) != VK_SUCCESS)
		return;

	Rt_ArrayAdd(&_modules, &info);
}

static int32_t
_sort(const struct ShaderModuleInfo *a, const struct ShaderModuleInfo *b)
{
	if (a->hash == b->hash)
		return 0;
	else if (a->hash > b->hash)
		return -1;
	else
		return 1;
}

static int32_t
_compare(const struct ShaderModuleInfo *item, const uint64_t *hash)
{
	if (item->hash == *hash)
		return 0;
	else if (item->hash > *hash)
		return -1;
	else
		return 1;
}*/
