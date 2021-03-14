#define Handle __EngineHandle

#include <System/System.h>
#include <Render/Device.h>
#include <Render/DescriptorSet.h>

#undef Handle

#include "MTLDriver.h"

#define APPEND_BINDING_COUNT(x) \
if ((b->stage & SS_ALL_GRAPHICS) == SS_ALL_GRAPHICS) {	\
	ds->vertex.x += b->count;							\
	ds->fragment.x += b->count;							\
} else {												\
	if ((b->stage & SS_VERTEX) == SS_VERTEX)			\
		ds->vertex.x += b->count;						\
	if ((b->stage & SS_FRAGMENT) == SS_FRAGMENT)		\
		ds->fragment.x += b->count;						\
}														\
if ((b->stage & SS_COMPUTE) == SS_COMPUTE)				\
	ds->compute.x += b->count

static inline bool _AllocateDescriptors(struct MTLDrvDescriptors *d);
static inline bool _LinkBindings(struct MTLDrvDescriptors *d, const struct DescriptorBinding *b, uint32_t *nextBinding, struct MTLDrvBinding *bindings);
static inline void _FreeDescriptors(struct MTLDrvDescriptors *d);

struct DescriptorSetLayout *
MTL_CreateDescriptorSetLayout(id<MTLDevice> dev, const struct DescriptorSetLayoutDesc *desc)
{
	struct DescriptorSetLayout *dsl = calloc(1, sizeof(*dsl));
	if (!dsl)
		return NULL;
	
	dsl->desc.bindingCount = desc->bindingCount;
	dsl->desc.bindings = calloc(sizeof(*dsl->desc.bindings), desc->bindingCount);
	memcpy(dsl->desc.bindings, desc->bindings, sizeof(*dsl->desc.bindings) * desc->bindingCount);
	
	return dsl;
}

void
MTL_DestroyDescriptorSetLayout(id<MTLDevice> dev, struct DescriptorSetLayout *dsl)
{
	free(dsl->desc.bindings);
	free(dsl);
}

struct DescriptorSet *
MTL_CreateDescriptorSet(id<MTLDevice> dev, const struct DescriptorSetLayout *layout)
{
	struct DescriptorSet *ds = calloc(1, sizeof(*ds));
	if (!ds)
		return NULL;
	
	ds->layout = layout;
	
	for (uint32_t i = 0; i < layout->desc.bindingCount; ++i) {
		ds->bindingCount += layout->desc.bindings[i].count;
		
		const struct DescriptorBinding *b = &layout->desc.bindings[i];
		if (b->type == DT_STORAGE_BUFFER || b->type == DT_UNIFORM_BUFFER || b->type == DT_ACCELERATION_STRUCTURE) {
			APPEND_BINDING_COUNT(bufferCount);
		} else if (b->type == DT_TEXTURE) {
			APPEND_BINDING_COUNT(textureCount);
		} else if (b->type == DT_SAMPLER) {
			APPEND_BINDING_COUNT(samplerCount);
		}
	}
	
	ds->bindings = calloc(sizeof(*ds->bindings), ds->bindingCount);
	if (!ds->bindings)
		goto error;
	
	if (!_AllocateDescriptors(&ds->vertex) || !_AllocateDescriptors(&ds->fragment) || !_AllocateDescriptors(&ds->compute))
		goto error;
	
	uint32_t nextBinding = 0;
	for (uint32_t i = 0; i < layout->desc.bindingCount; ++i) {
		const struct DescriptorBinding b = layout->desc.bindings[i];
		
		if ((b.stage & SS_ALL_GRAPHICS) == SS_ALL_GRAPHICS) {
			if (!_LinkBindings(&ds->vertex, &b, &nextBinding, ds->bindings))
				goto error;
			
			if (!_LinkBindings(&ds->fragment, &b, &nextBinding, ds->bindings))
				goto error;
		} else {
			if ((b.stage & SS_VERTEX) == SS_VERTEX)
				if (!_LinkBindings(&ds->vertex, &b, &nextBinding, ds->bindings))
					goto error;
			
			if ((b.stage & SS_FRAGMENT) == SS_FRAGMENT)
				if (!_LinkBindings(&ds->fragment, &b, &nextBinding, ds->bindings))
					goto error;
		}
		
		if ((b.stage & SS_COMPUTE) == SS_COMPUTE)
			if (!_LinkBindings(&ds->compute, &b, &nextBinding, ds->bindings))
				goto error;
	}
	
	return ds;

error:
	_FreeDescriptors(&ds->vertex);
	_FreeDescriptors(&ds->fragment);
	_FreeDescriptors(&ds->compute);
	
	free(ds->bindings);
	
	free(ds);
	
	return NULL;
}

void
MTL_WriteDescriptorSet(id<MTLDevice> dev, struct DescriptorSet *ds, const struct DescriptorWrite *writes, uint32_t writeCount)
{
	for (uint32_t i = 0; i < writeCount; ++i) {
		const struct DescriptorWrite w = writes[i];
		
		uint32_t first = w.binding;
		for (uint32_t i = 0; i < w.count; ++i)
		
		switch (w.type) {
		case DT_STORAGE_BUFFER:
		case DT_UNIFORM_BUFFER:
		case DT_ACCELERATION_STRUCTURE:
			for (uint32_t j = 0; j < w.count; ++j) {
				*ds->bindings[first + i].buffer.ptr = w.bufferInfo[j].buff->buff;
				*ds->bindings[first + i].buffer.offset = w.bufferInfo[j].offset;
			}
		break;
		case DT_TEXTURE:
			for (uint32_t j = 0; j < w.count; ++j)
				*ds->bindings[first + i].texture = w.textureInfo[j].tex->tex;
		break;
		case DT_SAMPLER:
			for (uint32_t j = 0; j < w.count; ++j)
				*ds->bindings[first + i].sampler = (id<MTLSamplerState>)w.samplers[j];
		}
	}
}

void
MTL_CopyDescriptorSet(id<MTLDevice> dev, const struct DescriptorSet *src, uint32_t srcOffset, struct DescriptorSet *dst, uint32_t dstOffset, uint32_t count)
{
	memcpy(&dst->bindings[dstOffset], &src->bindings[srcOffset], sizeof(*dst->bindings) * count);
}

void
MTL_DestroyDescriptorSet(id<MTLDevice> dev, struct DescriptorSet *ds)
{
	_FreeDescriptors(&ds->vertex);
	_FreeDescriptors(&ds->fragment);
	_FreeDescriptors(&ds->compute);
	
	free(ds->bindings);
	free(ds);
}

static inline bool
_AllocateDescriptors(struct MTLDrvDescriptors *d)
{
	if (d->bufferCount) {
		d->buffers = calloc(d->bufferCount, sizeof(*d->buffers));
		if (!d->buffers)
			return false;
	
		d->offsets = calloc(d->bufferCount, sizeof(*d->offsets));
		if (!d->offsets)
			return false;
	}
	
	if (d->textureCount) {
		d->textures = calloc(d->textureCount, sizeof(*d->textures));
		if (!d->textures)
			return false;
	}
	
	if (d->samplerCount) {
		d->samplers = calloc(d->samplerCount, sizeof(*d->samplers));
		if (!d->samplers)
			return false;
	}
	
	d->bufferCount = 0;
	d->textureCount = 0;
	d->samplerCount = 0;
	
	return true;
}

static inline bool
_LinkBindings(struct MTLDrvDescriptors *d, const struct DescriptorBinding *b, uint32_t *nextBinding, struct MTLDrvBinding *bindings)
{
	uint32_t next = *nextBinding;
	
	if (b->type == DT_UNIFORM_BUFFER || b->type == DT_STORAGE_BUFFER) {
		uint32_t nextBuffer = d->bufferCount;
		
		d->bufferCount += b->count;
		
		for (uint32_t i = 0; i < b->count; ++i) {
			bindings[next].buffer.offset = &d->offsets[nextBuffer];
			bindings[next++].buffer.ptr = &d->buffers[nextBuffer++];
		}
	} else if (b->type == DT_TEXTURE) {
		uint32_t nextTexture = d->textureCount;
		d->textureCount += b->count;
		
		for (uint32_t i = 0; i < b->count; ++i)
			bindings[next++].texture = &d->textures[nextTexture++];
	} else if (b->type == DT_SAMPLER) {
		uint32_t nextSampler = d->samplerCount;
		d->samplerCount += b->count;
		
		for (uint32_t i = 0; i < b->count; ++i)
			bindings[next++].sampler = &d->samplers[nextSampler++];
	}
	
	*nextBinding = next;
	
	return true;
}

static inline void
_FreeDescriptors(struct MTLDrvDescriptors *d)
{
	free(d->buffers); d->buffers = NULL;
	free(d->offsets); d->offsets = NULL;
	free(d->textures); d->textures = NULL;
	free(d->samplers); d->samplers = NULL;
	
	d->bufferCount = 0;
	d->textureCount = 0;
	d->samplerCount = 0;
}
