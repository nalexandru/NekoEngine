#include <Engine/Config.h>
#include <Runtime/Runtime.h>
#include <System/AtomicLock.h>
#include <Render/DescriptorSet.h>

#include "VulkanDriver.h"

struct VkDrvDescriptorAlloc
{
	VkDescriptorSet set;
	VkDescriptorPool pool;
};

struct VkDrvDescriptorPool
{
	const struct DescriptorSetLayout *dsl;
	uint32_t *allocCount;
	VkDescriptorPool *pools;
	struct Queue freeSets;
	struct Array allocations;
	uint32_t setsPerPool, poolCount, totalAlloc;
	VkDescriptorPoolCreateInfo ci;
};

static struct Array _pools;

static struct AtomicLock _lock;
static inline struct VkDrvDescriptorPool *_CreatePool(VkDevice dev, const struct DescriptorSetLayout *dsl);
static inline bool _GrowPool(struct VkDrvDescriptorPool *dp);
static int32_t _ComparePool(const struct VkDrvDescriptorPool *item, const VkDescriptorSetLayout layout);
static inline struct VkDrvDescriptorPool *_FindPool(VkDescriptorSet set);

struct DescriptorSetLayout *
Vk_CreateDescriptorSetLayout(struct RenderDevice *dev, const struct DescriptorSetLayoutDesc *desc)
{
	struct DescriptorSetLayout *dsl = calloc(1, sizeof(*dsl));
	if (!dsl)
		return NULL;

	dsl->sizeCount = desc->bindingCount;
	dsl->sizes = calloc(desc->bindingCount, sizeof(*dsl->sizes));
	if (!dsl->sizes) {
		free(dsl);
		return NULL;
	}

	VkDescriptorSetLayoutBinding *bindings = Sys_Alloc(sizeof(*bindings), desc->bindingCount, MH_Transient);
	VkDescriptorBindingFlags *bindingFlags = Sys_Alloc(sizeof(*bindingFlags), desc->bindingCount, MH_Transient);

	uint32_t nextBinding = 0;
	for (uint32_t i = 0; i < desc->bindingCount; ++i) {
		bindings[i].binding = nextBinding;

		switch (desc->bindings[i].type) {
		case DT_UNIFORM_BUFFER: bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
		case DT_STORAGE_BUFFER: bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
		case DT_TEXTURE: bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
		case DT_ACCELERATION_STRUCTURE: bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR; break;
		case DT_SAMPLER: bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		}

		bindings[i].descriptorCount = desc->bindings[i].count;
		bindings[i].stageFlags = desc->bindings[i].stage;

		dsl->sizes[i].type = bindings[i].descriptorType;
		dsl->sizes[i].descriptorCount = bindings[i].descriptorCount;

		//bindings[i].pImmutableSamplers = NULL;

		bindingFlags[i] = 0;

		if ((desc->bindings[i].flags & DBF_UPDATE_AFTER_BIND) == DBF_UPDATE_AFTER_BIND)
			bindingFlags[i] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

		if ((desc->bindings[i].flags & DBF_PARTIALLY_BOUND) == DBF_PARTIALLY_BOUND)
			bindingFlags[i] |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

		nextBinding += bindings[i].descriptorCount;
	}

	VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.bindingCount = desc->bindingCount,
		.pBindingFlags = bindingFlags
	};
	VkDescriptorSetLayoutCreateInfo info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.pNext = &flagsInfo,
		.bindingCount = desc->bindingCount,
		.pBindings = bindings
	};
	if (vkCreateDescriptorSetLayout(dev->dev, &info, Vkd_allocCb, &dsl->layout) != VK_SUCCESS) {
		free(dsl->sizes);
		free(dsl);
		return NULL;
	}

	return dsl;
}

void
Vk_DestroyDescriptorSetLayout(struct RenderDevice *dev, struct DescriptorSetLayout *dsl)
{
	vkDestroyDescriptorSetLayout(dev->dev, dsl->layout, Vkd_allocCb);
	free(dsl->sizes);
	free(dsl);
}

VkDescriptorSet
Vk_CreateDescriptorSet(struct RenderDevice *dev, const struct DescriptorSetLayout *dsl)
{
	Sys_AtomicLockWrite(&_lock);

	VkDescriptorSet set = VK_NULL_HANDLE;
	struct VkDrvDescriptorPool *dp = Rt_ArrayFind(&_pools, dsl->layout, (RtCmpFunc)_ComparePool);

	if (!dp) {
		dp = _CreatePool(dev->dev, dsl);
		if (!dp)
			return VK_NULL_HANDLE;
	} else {
		if (dp->freeSets.count > 0) {
			set = Rt_QueuePopPtr(&dp->freeSets);
			return set;
		}
	}

	uint32_t i = 0;
	while (true) {
		for (; i < dp->poolCount; ++i) {
			if (dp->allocCount[i] < dp->setsPerPool)
				break;
		}

		if (i == dp->poolCount) {
			if (!_GrowPool(dp))
				goto exit;

			continue;
		}

		VkDescriptorSetAllocateInfo ai =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = dp->pools[i],
			.descriptorSetCount = 1,
			.pSetLayouts = &dsl->layout
		};

		if (vkAllocateDescriptorSets(dev->dev, &ai, &set) != VK_SUCCESS)
			goto exit;

		break;
	}
	
	struct VkDrvDescriptorAlloc alloc = { .set = set, .pool = dp->pools[i] };
	Rt_ArrayAdd(&dp->allocations, &alloc);
	++dp->allocCount[i];

exit:
	Sys_AtomicUnlockWrite(&_lock);
	return set;
}

void
Vk_CopyDescriptorSet(struct RenderDevice *dev, const VkDescriptorSet src, uint32_t srcOffset, VkDescriptorSet dst, uint32_t dstOffset, uint32_t count)
{
	VkCopyDescriptorSet copy =
	{
		.srcSet = src,
		.srcBinding = srcOffset,
		.dstSet = dst,
		.dstBinding = dstOffset,
		.descriptorCount = count
	};
	vkUpdateDescriptorSets(dev->dev, 0, NULL, 1, &copy);
}

void
Vk_WriteDescriptorSet(struct RenderDevice *dev, VkDescriptorSet ds, const struct DescriptorWrite *writes, uint32_t writeCount)
{
	VkWriteDescriptorSet *wds = Sys_Alloc(sizeof(*wds), writeCount, MH_Transient);

	for (uint32_t i = 0; i < writeCount; ++i) {
		wds[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds[i].dstSet = ds;
		wds[i].dstBinding = writes[i].binding;
		wds[i].descriptorCount = writes[i].count;
		wds[i].dstArrayElement = writes[i].arrayElement;

		switch (writes[i].type) {
		case DT_STORAGE_BUFFER:
		case DT_UNIFORM_BUFFER: {
			wds[i].descriptorType = writes[i].type == DT_STORAGE_BUFFER ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			VkDescriptorBufferInfo *info = Sys_Alloc(sizeof(*info), wds[i].descriptorCount, MH_Transient);

			for (uint32_t j = 0; j < wds[i].descriptorCount; ++j) {
				info[j].buffer = writes[i].bufferInfo[j].buff->buff;
				info[j].offset = writes[i].bufferInfo[j].offset;
				info[j].range = writes[i].bufferInfo[j].size;
			}

			wds[i].pBufferInfo = info;
		} break;
		case DT_TEXTURE: {
			wds[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			VkDescriptorImageInfo *info = Sys_Alloc(sizeof(*info), wds[i].descriptorCount, MH_Transient);

			for (uint32_t j = 0; j < wds[i].descriptorCount; ++j) {
				info[j].imageView = writes[i].textureInfo[j].tex->imageView;
				info[j].imageLayout = NeToVkImageLayout(writes[i].textureInfo[j].layout);
			}

			wds[i].pImageInfo = info;
		} break;
		case DT_ACCELERATION_STRUCTURE: {
			VkWriteDescriptorSetAccelerationStructureKHR *as = Sys_Alloc(sizeof(*as), 1, MH_Transient);
			VkAccelerationStructureKHR *structures = Sys_Alloc(sizeof(*structures), writes[i].count, MH_Transient);
			for (uint32_t j = 0; j < writes[j].count; ++j)
				structures[j] = writes[j].accelerationStructureInfo->as->as;

			as->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
			as->pAccelerationStructures = structures;
			as->accelerationStructureCount = writes[i].count;
		} break;
		case DT_SAMPLER: {
			wds[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			VkDescriptorImageInfo *info = Sys_Alloc(sizeof(*info), wds[i].descriptorCount, MH_Transient);

			for (uint32_t j = 0; j < wds[i].descriptorCount; ++j)
				info[j].sampler = (VkSampler)writes[i].samplers[j];

			wds[i].pImageInfo = info;
		}
		}
	}

	vkUpdateDescriptorSets(dev->dev, writeCount, wds, 0, NULL);
}

void
Vk_DestroyDescriptorSet(struct RenderDevice *dev, VkDescriptorSet set)
{
	Sys_AtomicLockWrite(&_lock);

	struct VkDrvDescriptorPool *dp = _FindPool(set);

	Rt_QueuePushPtr(&dp->freeSets, set);

	Sys_AtomicUnlockWrite(&_lock);
}

bool
Vk_InitDescriptorPools(VkDevice dev)
{
	(void)dev;
	return Rt_InitArray(&_pools, 10, sizeof(struct VkDrvDescriptorPool));
}

void
Vk_TermDescriptorPools(VkDevice dev)
{
	struct VkDrvDescriptorPool *p;
	Rt_ArrayForEach(p, &_pools) {
		for (uint32_t i = 0; i < p->poolCount; ++i)
			vkDestroyDescriptorPool(dev, p->pools[i], Vkd_allocCb);
	}

	Rt_TermArray(&_pools);
}

static inline struct VkDrvDescriptorPool *
_CreatePool(VkDevice dev, const struct DescriptorSetLayout *dsl)
{
	uint32_t setsPerPool = E_GetCVarI32(L"VulkanDrv_DescriptorSetsPerPool", 10)->i32;
	uint32_t initialPoolCount = E_GetCVarI32(L"VulkanDrv_InitialDescriptorPoolCount", 1)->i32;

	struct VkDrvDescriptorPool pool =
	{
		.dsl = dsl,
		.setsPerPool = setsPerPool,
		.poolCount = initialPoolCount,
		.ci =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = setsPerPool,
			.poolSizeCount = dsl->sizeCount,
			.pPoolSizes = dsl->sizes
		}
	};

	pool.pools = calloc(initialPoolCount, sizeof(*pool.pools));
	if (!pool.pools)
		goto error;

	pool.allocCount = calloc(initialPoolCount, sizeof(*pool.allocCount));
	if (!pool.allocCount)
		goto error;

	if (!Rt_InitPtrQueue(&pool.freeSets, initialPoolCount * setsPerPool))
		goto error;

	if (!Rt_InitArray(&pool.allocations, initialPoolCount * setsPerPool, sizeof(struct VkDrvDescriptorAlloc)))
		goto error;

	for (uint32_t i = 0; i < initialPoolCount; ++i)
		vkCreateDescriptorPool(dev, &pool.ci, Vkd_allocCb, &pool.pools[i]);

	Rt_ArrayAdd(&_pools, &pool);
	return (struct VkDrvDescriptorPool *)Rt_ArrayLast(&_pools);

error:
	free(pool.pools);
	free(pool.allocCount);
	Rt_TermQueue(&pool.freeSets);
	Rt_TermArray(&pool.allocations);

	return NULL;
}

static inline bool
_GrowPool(struct VkDrvDescriptorPool *dp)
{
	return false;
}

static int32_t
_ComparePool(const struct VkDrvDescriptorPool *item, const VkDescriptorSetLayout layout)
{
	if (item->dsl->layout == layout)
		return 0;
	else
		return 1;
}

static inline struct VkDrvDescriptorPool *
_FindPool(VkDescriptorSet set)
{
	for (size_t i = 0; i < _pools.count; ++i) {
		struct VkDrvDescriptorAlloc *a;
		struct VkDrvDescriptorPool *p = Rt_ArrayGet(&_pools, i);

		Rt_ArrayForEach(a, &p->allocations)
			if (a->set == set)
				return p;
	}

	return NULL;
}

