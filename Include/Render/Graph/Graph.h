#ifndef NE_RENDER_GRAPH_GRAPH_H
#define NE_RENDER_GRAPH_GRAPH_H

#include <Render/Types.h>
#include <Render/Core.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RE_OUTPUT					"Re_output"
#define RE_DEPTH_BUFFER				"Re_depthBuffer"
#define RE_NORMAL_BUFFER			"Re_normalBuffer"
#define RE_AO_BUFFER				"Re_aoBuffer"

#define RE_VISIBLE_LIGHT_INDICES	"Re_visibleLightIndices"

#define RE_SCENE_DATA				"Scn_data"
#define RE_SCENE_INSTANCES			"Scn_instances"

#define RE_CAMERA					"Re_camera"

#define RE_PASS_SEMAPHORE			"Re_passSemaphore"

extern struct NeRenderGraph *Re_activeGraph;

bool Re_AddGraphTexture(const char *name, const struct NeTextureDesc *desc, uint16_t location, struct NeArray *resources);
bool Re_AddGraphBuffer(const char *name, const struct NeBufferDesc *desc, struct NeArray *resources);
bool Re_AddGraphData(const char *name, void *ptr, struct NeArray *resources);

struct NeTexture *Re_GraphTexture(uint64_t hash, const struct NeArray *resources, uint32_t *location, struct NeTextureDesc **desc);
uint16_t Re_GraphTextureLocation(uint64_t hash, const struct NeArray *resources);
struct NeTexture *Re_GraphTexturePtr(uint64_t hash, const struct NeArray *resources);
const struct NeTextureDesc *Re_GraphTextureDesc(uint64_t hash, const struct NeArray *resources);

uint64_t Re_GraphBuffer(uint64_t hash, const struct NeArray *resources, struct NeBuffer **buff);
void *Re_GraphData(uint64_t hash, const struct NeArray *resources);

struct NeRenderGraph *Re_CreateGraph(void);
struct NeRenderGraph *Re_CreateDefaultGraph(void);
bool Re_AddPass(struct NeRenderGraph *g, uint64_t name);
void Re_BuildGraph(struct NeRenderGraph *g, const struct NeTextureDesc *outputDesc, struct NeTexture *output);
void Re_ExecuteGraph(struct NeRenderGraph *g);
void Re_DestroyGraph(struct NeRenderGraph *g);

#ifdef __cplusplus
}
#endif

#endif /* NE_RENDER_GRAPH_GRAPH_H */

/* NekoEngine
 *
 * Graph.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
