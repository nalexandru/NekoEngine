#include "GLBackend.h"

// Function stubs for features that are not and will not be supported by OpenGL

struct NeAccelerationStructure *Re_CreateAccelerationStructure(const struct NeAccelerationStructureCreateInfo *info) { return NULL; }
uint64_t Re_AccelerationStructureBuildSize(const struct NeAccelerationStructure *as) { return 0; }
uint64_t Re_AccelerationStructureHandle(const struct NeAccelerationStructure *as) { return 0; }
void Re_DestroyAccelerationStructure(struct NeAccelerationStructure *as) { }

NeDirectIOHandle Re_BkOpenFile(const char *path) { return NULL; }
void Re_BkCloseFile(NeDirectIOHandle handle) { }

struct NePipeline *Re_BkRayTracingPipeline(const struct NeRayTracingPipelineDesc *desc) { return NULL; }

void Re_LoadPipelineCache(void) { }
void Re_SavePipelineCache(void) { }

void Re_CmdTraceRays(uint32_t width, uint32_t height, uint32_t depth) { }
void Re_CmdTraceRaysIndirect(struct NeBuffer *buff, uint64_t offset) { }
void Re_CmdBuildAccelerationStructures(uint32_t count, struct NeAccelerationStructureBuildInfo *buildInfo, const struct NeAccelerationStructureRangeInfo **rangeInfo) { }
void Re_CmdBarrier(enum NePipelineDependency dep, uint32_t memBarrierCount, const struct NeMemoryBarrier *memBarriers, uint32_t bufferBarrierCount, const struct NeBufferBarrier *bufferBarriers, uint32_t imageBarrierCount, const struct NeImageBarrier *imageBarriers) { }
void Re_CmdTransition(struct NeTexture *tex, enum NeTextureLayout newLayout) { }
void Re_CmdLoadBuffer(struct NeBuffer *buff, uint64_t offset, uint64_t size, void *handle, uint64_t sourceOffset) { }
void Re_CmdLoadTexture(struct NeTexture *tex, uint32_t slice, uint32_t level, uint32_t width, uint32_t height, uint32_t depth, uint32_t bytesPerRow, struct NeImageOffset *origin, void *handle, uint64_t sourceOffset) { }
void Re_BeginDirectIO(void) { }
bool Re_SubmitDirectIO(bool *completed) { return false; }
bool Re_ExecuteDirectIO(void) { return false; }
