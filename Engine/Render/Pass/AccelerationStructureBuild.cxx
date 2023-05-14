#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Engine/Engine.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/RayTracing.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

NE_RENDER_PASS(NeASBuild,
{
	void *unused;
});

static bool
NeASBuild_Setup(struct NeASBuild *pass, struct NeArray *resources)
{
	return true;
}

static void
NeASBuild_Execute(struct NeASBuild *pass, const struct NeArray *resources)
{
	Re_BeginComputeCommandBuffer(nullptr);

	struct NeAccelerationStructureGeometryDesc asg =
	{
		.type = ASG_INSTANCES,
		//.instances.address = 0 /* instance buffer address */
	};
	struct NeAccelerationStructureBuildInfo asbi =
	{
		.type = AS_TOP_LEVEL,
		.flags = ASF_FAST_BUILD,
		.mode = ASB_BUILD,
		.dst = nullptr, /* resource */
		.geometryCount = 0, /* instance count */
		.geometries = &asg,
		.scratchAddress = 0 /* scratch buffer */
	};

	Re_CmdBuildAccelerationStructures(1, &asbi, nullptr);

	struct NeSemaphore *passSemaphore = (struct NeSemaphore *)Re_GraphData(Rt_HashLiteral(RE_PASS_SEMAPHORE), resources);
	Re_QueueCompute(Re_EndCommandBuffer(), passSemaphore);
}

static bool
NeASBuild_Init(struct NeASBuild **pass)
{
	*pass = (struct NeASBuild *)Sys_Alloc(sizeof(struct NeASBuild), 1, MH_Render);
	return *pass != nullptr;
}

static void
NeASBuild_Term(struct NeASBuild *pass)
{
	Sys_Free(pass);
}

/* NekoEngine
 *
 * AccelerationStructureBuild.cxx
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
