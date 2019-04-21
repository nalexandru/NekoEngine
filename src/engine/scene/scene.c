/* NekoEngine
 *
 * scene.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Scene
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

#include <system/log.h>

#include <ecs/ecsys.h>
#include <scene/scene.h>
#include <scene/camera.h>
#include <scene/transform.h>
#include <engine/resource.h>
#include <engine/components.h>
#include <engine/math.h>
#include <graphics/mesh.h>
#include <graphics/graphics.h>
#include <graphics/drawable.h>

#define SCENE_MODULE	"Scene"

struct ne_scene
{
	bool loaded;
	rt_string name;
	rt_string path;
};

void scn_transform_sys(double, void **);
void scn_cull_system(double, void **);

ne_status
scn_sys_init(void)
{
	comp_type_id scn_comp[2] =
	{
		comp_get_type_id(NE_TRANSFORM_COMP_TYPE),
		comp_get_type_id(DRAWABLE_MESH_COMP_TYPE)
	};

	comp_type_id cam_comp = comp_get_type_id(CAMERA_COMP_TYPE);

	ecsys_register("scn_transform_sys", ECSYS_GROUP_LOGIC,
			scn_comp, 1, scn_transform_sys, false, ECSYS_PRI_CULLING);

	ecsys_register("scn_cull_sys", ECSYS_GROUP_RENDER,
			scn_comp, 2, scn_cull_system, false, ECSYS_PRI_CULLING);

	ecsys_register("cam_update_view_sys", ECSYS_GROUP_PRE_RENDER,
			&cam_comp, 1, camera_view_system, true, ECSYS_PRI_CAM_VIEW);

	ecsys_register("cam_update_proj_sys", ECSYS_GROUP_MANUAL,
			&cam_comp, 1, camera_proj_system, true, 0);

	return NE_OK;
}

void
scn_sys_release(void)
{
}

struct ne_scene *
scn_init(const char *path)
{
	struct ne_scene *ret = calloc(1, sizeof(*ret));
	if (!ret)
		return NULL;

	rt_string_init_with_cstr(&ret->path, path);

	// read header

	return ret;
}

ne_status
scn_load(struct ne_scene *scn)
{
	return NE_FAIL;
}

const char *
scn_get_name(struct ne_scene *scn)
{
	return scn->name.data;
}

bool
scn_is_loaded(struct ne_scene *scn)
{
	return false;
}

void
scn_update(struct ne_scene *scn)
{
	//
}

void
scn_cull_drawables(struct ne_scene *scn)
{
	//
}

void
scn_get_visible_drawables(struct ne_scene *scn,
	rt_array *dst)
{
	//ne_drawable d;
	//memset(&d, 0x0, sizeof(d));
}

void
scn_destroy(struct ne_scene *scn)
{
	if (!scn)
		return;

	rt_string_release(&scn->name);
	rt_string_release(&scn->path);

	free(scn);
}

void
scn_transform_sys(double dt,
	void **comp)
{
	//
}

void
scn_cull_system(
	double dt,
	void **comp)
{
	struct ne_transform_comp *t_comp = comp[0];
	struct ne_drawable_mesh_comp *m_comp = comp[1];
	kmMat4 tmp;

	// check if there are any visible drawables

	// update model data

	kmMat4Identity(&m_comp->data.model);
	kmMat4Translation(&m_comp->data.model, t_comp->pos.x, t_comp->pos.y, t_comp->pos.z);
//	kmMat4Scaling(&tmp, .05f, .05f, .05f);

//	kmMat4Multiply(&m_comp->data.model, &m_comp->data.model, &tmp);

	kmMat4Identity(&tmp);
	kmMat4Transpose(&m_comp->data.normal, kmMat4Inverse(&tmp, &m_comp->data.model));

	for (size_t i = 0; i < m_comp->mesh->groups.count; ++i) {
		gfx_add_mesh(m_comp, i);
	}
}
