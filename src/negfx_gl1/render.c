/* NekoEngine
 *
 * render.c
 * Author: Alexandru Naiman
 *
 * NekoEngine OpenGL 1 Graphics Subsystem
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
#include <scene/camera.h>
#include <runtime/runtime.h>
#include <graphics/mesh.h>
#include <graphics/vertex.h>
#include <graphics/texture.h>
#include <graphics/graphics.h>

#include <gl1gfx.h>
#include <buffer.h>
#include <texture.h>

#define GL1_RENDER_MODULE	"GL1_Render"

GLuint _dl = 0;

rt_array _display_lists;

struct gl1_mesh_info
{
	GLuint *textures;
	GLuint *targets;
	GLuint *lists;
};

struct gl1_draw_info
{
	GLuint texture;
	GLuint target;
	GLuint list;
	const struct ne_mesh_data *data;
};

static rt_array _lights;

ne_status
gl1gfx_init_drawable_mesh(struct ne_drawable_mesh_comp *comp)
{
	struct gl1_mesh_info *mi = (struct gl1_mesh_info *)&comp->private;

	mi->lists = calloc(comp->mesh->groups.count, sizeof(GLuint));
	mi->textures = calloc(comp->mesh->groups.count, sizeof(GLuint));
	mi->targets = calloc(comp->mesh->groups.count, sizeof(GLuint));

	for (size_t i = 0; i < comp->mesh->groups.count; ++i) {
		mi->lists[i] = glGenLists(1);

		struct ne_material *mat = rt_array_get_ptr(&comp->materials, i);

		glNewList(mi->lists[i], GL_COMPILE);
		glBegin(GL_TRIANGLES);
		{
			struct ne_mesh_group *grp =
				rt_array_get(&comp->mesh->groups, i);

			if (mat && mat->diffuse_map) {
				mi->textures[i] = mat->diffuse_map->id;
				mi->targets[i] = mat->diffuse_map->target;
			}

			uint32_t *idx = ((uint32_t *)comp->mesh->idx_buffer->ptr) +
					grp->idx_offset;
			struct ne_vertex *vtx = comp->mesh->vtx_buffer->ptr;
			vtx += grp->vtx_offset;

			for (uint32_t j = 0; j < grp->idx_count; ++j) {
				glColor4f(.8f, .8f, .8f, 1.f);
				glTexCoord2f(vtx[idx[j]].uv.x,
					vtx[idx[j]].uv.y);
				glNormal3f(vtx[idx[j]].normal.x,
					vtx[idx[j]].normal.y,
					vtx[idx[j]].normal.z);
				glVertex3f(vtx[idx[j]].pos.x,
					vtx[idx[j]].pos.y,
					vtx[idx[j]].pos.z);
			}
		}
		glEnd();
		glEndList();
	}

	return NE_OK;
}

void
gl1gfx_release_drawable_mesh(struct ne_drawable_mesh_comp *comp)
{
	struct gl1_mesh_info *mi = (struct gl1_mesh_info *)&comp->private;
	free(mi->lists);
}

void
gl1gfx_add_mesh(
	struct ne_drawable_mesh_comp *comp,
	size_t group)
{
	struct gl1_mesh_info *mi = (struct gl1_mesh_info *)&comp->private;
	struct gl1_draw_info di =
	{
		mi->textures[group],
		mi->targets[group],
		mi->lists[group],
		&comp->data
	};
	rt_array_add(&_display_lists, &di);
}

ne_status
gl1gfx_init_render(void)
{
	GLint max_lights = 0;
	glGetIntegerv(GL_MAX_LIGHTS, &max_lights);

	gl1gfx_module.max_lights = max_lights;

	rt_array_init(&_lights, max_lights, sizeof(struct ne_light));
	rt_array_init(&_display_lists, 10, sizeof(struct gl1_draw_info));

	return NE_OK;
}

void
gl1gfx_render_scene(void)
{
	kmMat4 vp;
	struct ne_light *light = NULL;
	struct gl1_draw_info *di = NULL;

	if (gl1gfx_view_matrix != MAT_PERSPECTIVE) {
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(ne_main_camera->proj.mat);

		gl1gfx_view_matrix = MAT_PERSPECTIVE;
	}

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	rt_array_foreach(light, &_lights) {
		if (light->pos.w == 0.f) {
			// Directional
			GLfloat dir[4] =
			{
				-light->dir.x,
				light->dir.y,
				light->dir.z,
				0.f
			};
			glLightfv(GL_LIGHT0/* + (GLenum)light->info.x*/,
					GL_POSITION, dir);
		}
	//	glLightf(light->info.x, GL_POSITION,
	}

	glMatrixMode(GL_MODELVIEW);

	rt_array_foreach(di, &_display_lists) {
		glPushMatrix();
		glLoadIdentity();

		kmMat4Multiply(&vp, &ne_main_camera->view, &di->data->model);
		glLoadMatrixf(vp.mat);

		if (di->texture)
			glBindTexture(di->target, di->texture);

		glCallList(di->list);

		if (di->texture)
			glBindTexture(di->target, 0);

		glPopMatrix();
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	_display_lists.count = 0;
}

struct ne_light *
gl1gfx_create_light(void)
{
	if (_lights.count == gl1gfx_module.max_lights)
		return NULL;

	return rt_array_create(&_lights);
}

void
gl1gfx_destroy_light(struct ne_light *l)
{
}

void
gl1gfx_release_render(void)
{
	rt_array_release(&_display_lists);
}
