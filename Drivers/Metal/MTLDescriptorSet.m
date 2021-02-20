#define Handle __EngineHandle

#include <Render/Device.h>
#include <Render/DescriptorSet.h>

#undef Handle

#include "MTLDriver.h"

struct DescriptorSetLayout *
MTL_CreateDescriptorSetLayout(id<MTLDevice> dev, const struct DescriptorSetLayoutDesc *desc)
{
	struct DescriptorSetLayout *dsl = calloc(1, sizeof(*dsl));
	
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
	ds->bindings = calloc(layout->desc.bindingCount, sizeof(*ds->bindings));
	if (!ds->bindings)
		goto error;
	
	for (uint32_t i = 0; i < layout->desc.bindingCount; ++i) {
		ds->bindings[i].count = layout->desc.bindings[i].count;
		
		if (layout->desc.bindings[i].type == DT_TEXTURE) {
			ds->bindings[i].first = ds->textureCount;
			ds->textureCount += ds->bindings[i].count;
		} else {
			ds->bindings[i].first = ds->bufferCount;
			ds->bufferCount += ds->bindings[i].count;
		}
	}
	
	ds->buffers = calloc(ds->bufferCount, sizeof(*ds->buffers));
	if (!ds->buffers)
		goto error;
		
	ds->textures = calloc(ds->textureCount, sizeof(*ds->textures));
	if (!ds->textures)
		goto error;
	
	return ds;

error:
	free(ds->buffers);
	free(ds->textures);
	free(ds->bindings);
	free(ds);
	
	return NULL;
}

void
MTL_WriteDescriptorSet(id<MTLDevice> dev, struct DescriptorSet *ds, const struct DescriptorWrite *writes, uint32_t writeCount)
{
	for (uint32_t i = 0; i < writeCount; ++i) {
		const struct DescriptorWrite w = writes[i];
		const struct MTLDrvBinding b = ds->bindings[w.binding];
		
		switch (w.type) {
		case DWT_BUFFER:
		case DWT_ACCELERATION_STRUCTURE:
			for (uint32_t j = 0; j < w.count; ++j) {
				ds->buffers[b.first + j].buffer = w.bufferInfo[j].buff->buff;
			}
		break;
		case DWT_TEXTURE:
		break;
		}
	}
}

void
MTL_DestroyDescriptorSet(id<MTLDevice> dev, struct DescriptorSet *ds)
{
	free(ds->bindings);
	free(ds->textures);
	free(ds->buffers);
	free(ds);
}
