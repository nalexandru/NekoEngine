#ifndef _RE_ACCELERATION_STRUCTURE_H_
#define _RE_ACCELERATION_STRUCTURE_H_

#include <Engine/Types.h>
#include <Render/Device.h>

struct AccelerationStructure;

struct AccelerationStructureDesc
{
	void *a;
};

struct AccelerationStructureCreateInfo
{
	struct AccelerationStructureDesc desc;
};

static inline struct Texture *Re_CreateAccelerationStructure(struct RenderDevice *dev, const struct AccelerationStructureCreateInfo *aci) { return Re_DeviceProcs.CreateAccelerationStructure(dev, aci); };
static inline void Re_DestroyAccelerationStructure(struct RenderDevice *dev, struct AccelerationStructure *as) { Re_DeviceProcs.DestroyAccelerationStructure(dev, as); }

//static inline void Re_UpdateTexture(struct Texture *tex, uint64_t offset, uint64_t size, void *data);

static inline const struct AccelerationStructureDesc *Re_AccelerationStructureDesc(const struct AccelerationStructure *as) { return NULL; }

#endif /* _RE_ACCELERATION_STRUCTURE_H_ */
