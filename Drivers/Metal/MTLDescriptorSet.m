#define Handle __EngineHandle

#include <System/System.h>
#include <Render/Device.h>
#include <Render/DescriptorSet.h>

#undef Handle

#include "MTLDriver.h"

static inline bool _AllocateDescriptors(struct MTLDrvDescriptors *d);
static inline bool _LinkBindings(struct MTLDrvDescriptors *d, const struct DescriptorBinding *b, uint32_t *nextBinding, struct MTLDrvBinding *bindings);
static inline void _FreeDescriptors(struct MTLDrvDescriptors *d);

struct DescriptorSetLayout *
MTL_CreateDescriptorSetLayout(id<MTLDevice> dev, const struct DescriptorSetLayoutDesc *desc)
{
	struct DescriptorSetLayout *dsl = calloc(1, sizeof(*dsl));
	if (!dsl)
		return NULL;
	
	memcpy(&dsl->desc, desc, sizeof(dsl->desc));
	
	return dsl;
}

void
MTL_DestroyDescriptorSetLayout(id<MTLDevice> dev, struct DescriptorSetLayout *dsl)
{
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
		if (b->type == DT_BUFFER) {
			if (b->stage & SS_ALL_GRAPHICS) {
				ds->vertex.bufferCount += b->count;
				ds->fragment.bufferCount += b->count;
			} else {
				if (b->stage & SS_VERTEX)
					ds->vertex.bufferCount += b->count;
				if (b->stage & SS_FRAGMENT)
					ds->fragment.bufferCount += b->count;
			}
			if (b->stage & SS_COMPUTE)
				ds->compute.bufferCount += b->count;
		} else if (b->type == DT_TEXTURE) {
			if (b->stage & SS_ALL_GRAPHICS) {
				ds->vertex.textureCount += b->count;
				ds->fragment.textureCount += b->count;
			} else {
				if (b->stage & SS_VERTEX)
					ds->vertex.textureCount += b->count;
				if (b->stage & SS_FRAGMENT)
					ds->fragment.textureCount += b->count;
			}
			if (b->stage & SS_COMPUTE)
				ds->compute.textureCount += b->count;
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
		
		if (b.stage & SS_ALL_GRAPHICS) {
			if (!_LinkBindings(&ds->vertex, &b, &nextBinding, ds->bindings))
				goto error;
			
			if (!_LinkBindings(&ds->fragment, &b, &nextBinding, ds->bindings))
				goto error;
		} else {
			if (b.stage & SS_VERTEX)
				if (!_LinkBindings(&ds->vertex, &b, &nextBinding, ds->bindings))
					goto error;
			
			if (b.stage & SS_FRAGMENT)
				if (!_LinkBindings(&ds->fragment, &b, &nextBinding, ds->bindings))
					goto error;
		}
		
		if (b.stage & SS_COMPUTE)
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
		case DWT_BUFFER:
		case DWT_ACCELERATION_STRUCTURE:
			for (uint32_t j = 0; j < w.count; ++j) {
				*ds->bindings[first + i].buffer.ptr = w.bufferInfo[i].buff->buff;
				*ds->bindings[first + i].buffer.offset = w.bufferInfo[i].offset;
			}
		break;
		case DWT_TEXTURE:
			for (uint32_t j = 0; j < w.count; ++j)
			*ds->bindings[first + i].texture = w.textureInfo[i].tex->tex;
		break;
		}
	}
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
	
	d->bufferCount = 0;
	d->textureCount = 0;
	
	return true;
}

static inline bool
_LinkBindings(struct MTLDrvDescriptors *d, const struct DescriptorBinding *b, uint32_t *nextBinding, struct MTLDrvBinding *bindings)
{
	uint32_t next = *nextBinding;
	
	if (b->type == DT_BUFFER) {
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
	
	d->bufferCount = 0;
	d->textureCount = 0;
}
