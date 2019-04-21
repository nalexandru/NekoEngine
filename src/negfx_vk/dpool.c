/* NekoEngine
 *
 * dpool.c
 * Author: Alexandru Naiman
 *
 * Vulkan Graphics Subsystem Descriptor Pool
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdlib.h>

#include <system/log.h>
#include <system/compat.h>
#include <runtime/runtime.h>

#include <dpool.h>
#include <vkgfx.h>
#include <vkutil.h>

#define DPOOL_MODULE	"Vulkan_DescriptorPool"

struct vkgfx_dpool_alloc
{
	VkDescriptorSet set;
	VkDescriptorPool pool;
};

struct vkgfx_dpool
{
	uint32_t *alloc_count;
	VkDescriptorPool *pools;
	rt_queue free_sets;
	struct vkgfx_dpool_alloc *allocs;
	uint32_t sets_per_pool;
	uint32_t pool_count;
	uint32_t total_allocs;
	VkDescriptorPoolCreateInfo ci;
	VkDescriptorPoolSize *sizes;
};

static int32_t _next_pool = 0;
static struct vkgfx_dpool _pools[10];

static inline ne_status
_dpool_grow(struct vkgfx_dpool *p)
{
	void *old_alloc_count = p->alloc_count;
	void *old_pools = p->pools;
	void *old_allocs = p->allocs;

	p->alloc_count = reallocarray(p->alloc_count,
			p->pool_count + 1, sizeof(uint32_t));
	if (!p->alloc_count)
		goto error;

	p->pools = reallocarray(p->pools,
			p->pool_count + 1, sizeof(VkDescriptorPool));
	if (!p->pools)
		goto error;

	p->allocs = reallocarray(p->allocs,
			(p->pool_count + 1) * p->sets_per_pool,
			sizeof(struct vkgfx_dpool_alloc));
	if (!p->allocs)
		goto error;

	VkResult res;
	VkDescriptorPool pool;
	res = vkCreateDescriptorPool(vkgfx_device, &p->ci,
				vkgfx_allocator, &pool);
	if (res != VK_SUCCESS) {
		log_entry(DPOOL_MODULE, LOG_CRITICAL,
			"Failed to create descriptor pool: %s",
			vku_result_string(res));
		goto error;
	}

	p->alloc_count[p->pool_count] = 0;
	p->pools[p->pool_count++] = pool;

	return NE_OK;

error:
	p->alloc_count = old_alloc_count;
	p->pools = old_pools;
	p->allocs = old_allocs;

	return NE_ALLOC_FAIL;
}

static inline struct vkgfx_dpool *
_validate_pool(int32_t type)
{
	if (type < 0 || type > 10)
		return NULL;

	struct vkgfx_dpool *p = &_pools[type];

	if (!p->pools)
		return NULL;

	return p;
}

int32_t
vkgfx_register_dp_type(
	const VkDescriptorPoolSize *sizes,
	uint32_t size_count,
	uint32_t sets_per_pool,
	uint32_t initial_pool_count)
{
	uint32_t id = _next_pool++;

	_pools[id].alloc_count = calloc(initial_pool_count, sizeof(uint32_t));
	if (!_pools[id].alloc_count)
		goto error;

	_pools[id].pools = calloc(initial_pool_count, sizeof(VkDescriptorPool));
	if (!_pools[id].pools)
		goto error;

	_pools[id].allocs = calloc(initial_pool_count * sets_per_pool,
			sizeof(struct vkgfx_dpool_alloc));
	if (!_pools[id].allocs)
		goto error;

	_pools[id].sizes = calloc(size_count, sizeof(VkDescriptorPoolSize));
	if (!_pools[id].sizes)
		goto error;

	memcpy(_pools[id].sizes, sizes, sizeof(VkDescriptorPoolSize) * size_count);

	_pools[id].ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	_pools[id].ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	_pools[id].ci.maxSets = sets_per_pool;
	_pools[id].ci.poolSizeCount = size_count;
	_pools[id].ci.pPoolSizes = _pools[id].sizes;

	for (uint32_t i = 0; i < initial_pool_count; ++i) {
		VkResult res;
		VkDescriptorPool p;
		res = vkCreateDescriptorPool(vkgfx_device, &_pools[id].ci,
					vkgfx_allocator, &p);
		if (res != VK_SUCCESS) {
			log_entry(DPOOL_MODULE, LOG_CRITICAL,
				"Failed to create descriptor pool: %s",
				vku_result_string(res));
			goto error;
		}

		_pools[id].pools[i] = p;
	}

	_pools[id].sets_per_pool = sets_per_pool;
	_pools[id].pool_count = initial_pool_count;

	if (rt_queue_init(&_pools[id].free_sets, sets_per_pool,
				sizeof(VkDescriptorSet)) != SYS_OK)
		goto error;

	return id;

error:
	if (_pools[id].pools) {
		for (uint32_t i = 0; i < initial_pool_count; ++i) {
			VkDescriptorPool p = _pools[id].pools[i];
			if (p != VK_NULL_HANDLE)
				vkDestroyDescriptorPool(vkgfx_device, p, vkgfx_allocator);
		}
	}

	free(_pools[id].pools);
	free(_pools[id].allocs);
	free(_pools[id].sizes);
	free(_pools[id].alloc_count);

	memset(&_pools[id], 0x0, sizeof(struct vkgfx_dpool));

	return -1;
}

ne_status
vkgfx_ds_alloc(
	int32_t type,
	VkDescriptorSet *set,
	VkDescriptorSetLayout layout)
{
	VkResult res;

	struct vkgfx_dpool *p = _validate_pool(type);
	if (!p)
		return NE_INVALID_ARGS;

	if (p->free_sets.count > 0) {
		*set = rt_queue_pop_ptr(&p->free_sets);
		return NE_OK;
	}

	uint32_t i = 0;

	while (true) {
		for (; i < p->pool_count; ++i) {
			if (p->alloc_count[i] < p->sets_per_pool)
				break;
		}

		if (i == p->pool_count) {
			ne_status ret = _dpool_grow(p);
			
			if (ret != NE_OK)
				return ret;

			continue;
		}

		VkDescriptorSetAllocateInfo ai;
		memset(&ai, 0x0, sizeof(ai));
		ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		ai.descriptorPool = p->pools[i];
		ai.descriptorSetCount = 1;
		ai.pSetLayouts = &layout;

		// TODO: check VK_ERROR_FRAGMENTED_POOL
		res = vkAllocateDescriptorSets(vkgfx_device, &ai, set);

		if (res == VK_SUCCESS)
			break;
	}

	p->allocs[p->total_allocs].set = *set;
	p->allocs[p->total_allocs++].pool = p->pools[i];
	++p->alloc_count[i];

	return NE_OK;
}

ne_status
vkgfx_ds_free(
	int32_t type,
	VkDescriptorSet set)
{
	struct vkgfx_dpool *p = _validate_pool(type);
	if (!p)
		return NE_INVALID_ARGS;

	rt_queue_push_ptr(&p->free_sets, set);

	return NE_OK;
}

ne_status
vkgfx_init_dpool(void)
{
	memset(&_pools, 0x0, sizeof(_pools));
	return NE_OK;
}

void
vkgfx_release_dpool(void)
{
	for (uint8_t i = 0; i < _next_pool; ++i) {
		for (uint32_t j = 0; j < _pools[i].pool_count; ++j)
			vkDestroyDescriptorPool(vkgfx_device, _pools[i].pools[j], vkgfx_allocator);
		free(_pools[i].pools);
		free(_pools[i].alloc_count);
		free(_pools[i].allocs);
		free(_pools[i].sizes);
		rt_queue_release(&_pools[i].free_sets);
	}
}

