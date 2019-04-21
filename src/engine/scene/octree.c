/* NekoEngine
 *
 * octree.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Octree
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

#include <scene/octree.h>
#include <scene/bounds.h>

#define NOC_GROW_MAX_ITER		40

struct ne_octree_node
{
	kmVec3 center;
	ne_bounds bounds;
	uint64_t count;
	uint64_t max_obj;
	struct ne_octree_node *children;
	rt_array objects;
	float length;
	float min_size;
	float looseness;
	float loose_len;
};
typedef struct ne_octree_node ne_octree_node;

struct ne_octree
{
	ne_octree_node *root;
	uint64_t count;
	float looseness;
	float min_size;
};

// uint32_t _GetBestFit(const NBounds &bounds) const { return (bounds.GetCenter().x <= _center.x ? 0 : 1) + (bounds.GetCenter().y >= _center.y ? 0 : 4) + (bounds.GetCenter().z <= _center.z ? 0 : 2); }

static int32_t
_obj_cmp_func(
	const void *item,
	const void *data)
{
	return !(item == data);
}

void
_octree_node_init(
	ne_octree_node *node,
	kmVec3 *center,
	float length,
	float min_size,
	float looseness,
	uint64_t max_obj)
{
	if (!node)
		return;

	memset(node, 0x0, sizeof(*node));

	memcpy(&node->center, center, sizeof(node->center));

	node->looseness = looseness;
	node->max_obj = max_obj;
	node->length = length;
	node->min_size = min_size;

	node->loose_len = length * looseness;

	rt_array_init_ptr(&node->objects, node->max_obj);

	//bounds_set_box(&node->bounds, )
	
	// init bounds
	// bounds(vec3(-node->loose_len), vec3(node->loose_len))
	// bounds.center(node->center)
}

bool
_octree_node_remove(
	ne_octree_node *node,
	const void *obj)
{
	size_t id = 0;

	id = rt_array_find_id(&node->objects, obj, _obj_cmp_func);

	if (id != RT_NOT_FOUND) {
		rt_array_remove(&node->objects, id);

		// attempt merge

		/*
		
		        if (!_children)
                return;

        size_t count{ _objects.Count() };

        for (uint8_t i = 0; i < 8; ++i)
        {
                if (_children[i]._children)
                        return;
                count += _children[i]._objects.Count();
        }

        if (count >= _maxObjects)
                return;

        for (uint8_t i = 0; i < 8; ++i)
                _objects.Add(_children[i]._objects);

        delete[] _children;
        _children = nullptr;
		
		*/

		return true;
	}

	return false;
}

bool
_octree_node_add(
	ne_octree_node *node,
	const void *obj)
{
	float quat = 0.f, half = 0.f;
	kmVec3 center;
	rt_array removed;

	// check obj transformed bounds
	// check node bounds contain

	if (node->count < node->max_obj || (node->length / 2) < node->min_size) {
		rt_array_add_ptr(&node->objects, obj);
		++node->count;
		return true;
	}

	if (!node->children) {
		quat = node->length / 4.f;
		half = node->length / 2.f;

		node->children = calloc(8, sizeof(ne_octree_node));
		if (!node->children)
			return false;

		kmVec3Fill(&center, -quat, -quat, quat);
		_octree_node_init(&node->children[0], &center, half,
			node->min_size, node->looseness, 20);

		kmVec3Fill(&center, quat, quat, -quat);
		_octree_node_init(&node->children[1], &center, half,
			node->min_size, node->looseness, 20);

		kmVec3Fill(&center, -quat, quat, quat);
		_octree_node_init(&node->children[2], &center, half,
			node->min_size, node->looseness, 20);

		kmVec3Fill(&center, quat, quat, quat);
		_octree_node_init(&node->children[3], &center, half,
			node->min_size, node->looseness, 20);

		kmVec3Fill(&center, -quat, -quat, -quat);
		_octree_node_init(&node->children[4], &center, half,
			node->min_size, node->looseness, 20);

		kmVec3Fill(&center, quat, -quat, -quat);
		_octree_node_init(&node->children[5], &center, half,
			node->min_size, node->looseness, 20);

		kmVec3Fill(&center, -quat, -quat, quat);
		_octree_node_init(&node->children[6], &center, half,
			node->min_size, node->looseness, 20);

		kmVec3Fill(&center, quat, -quat, quat);
		_octree_node_init(&node->children[6], &center, half,
			node->min_size, node->looseness, 20);

		rt_array_init_ptr(&removed, node->max_obj);

		/*
		check if objects fit in children
		for (const Object *obj : _objects)
			if (_children[_GetBestFit(obj->GetTransformedBounds())].Add(obj))
				removedObjects.Add(obj);		
		*/

		for (size_t i = 0; i < removed.count; ++i)
			_octree_node_remove(node, rt_array_get_ptr(&removed, i));
	}

	// insert into best fit child

	return false;
}

void
_octree_node_shrink(ne_octree_node *node)
{
	//
}

void
_octree_node_get_visible(
	ne_octree_node *node,
	const void *frustum,
	rt_array *dst)
{
	/*
	
	if (!frustum.ContainsBounds(_bounds))
				return;

		for (const Object *obj : _objects)
		{
				if (obj->GetNoCull() || !obj->IsVisible() || !frustum.ContainsBounds(obj->GetTransformedBounds()))
						continue;

				visibleObjects.Add(obj);
		}

		if (!_children)
				return;

		for (uint8_t i = 0; i < 8; ++i)
				_children[i].GetVisible(frustum, visibleObjects);
	
	*/
}

void
_octree_node_get_coliding(
	ne_octree_node *node,
	const void *frustum,
	rt_array *dst)
{
	/*
	
	        if (!_bounds.Intersects(bounds))
                return;

        for (const Object *obj : _objects)
                if (obj->GetTransformedBounds().Intersects(bounds))
                        collidingObjects.Add(obj);

        if (!_children)
                return;

        for (uint8_t i = 0; i < 8; ++i)
                _children[i].GetColliding(bounds, collidingObjects);
	
	*/
}

void
_octree_node_destroy(ne_octree_node *node)
{
	//
}

// octree

ne_octree *
octree_init(
	kmVec3 *center,
	float init_size,
	float looseness,
	float min_size)
{
	ne_octree *oct = calloc(1, sizeof(*oct));
	if (!oct)
		return NULL;

	oct->root = malloc(sizeof(*oct->root));
	if (!oct->root)
		return NULL;

	_octree_node_init(oct->root, center, init_size, min_size, looseness, 20);

	oct->count = 0;
	oct->looseness = looseness;
	oct->min_size = min_size;

	return oct;
}

static inline void
_octree_grow(
	ne_octree *oct,
	kmVec3 *dir)
{
	int x_dir = 0, y_dir = 0, z_dir = 0,
		root_pos;
	ne_octree_node *old_root = oct->root;
	float half = oct->root->length / 2, len = oct->root->length * 2;
	kmVec3 center, node_center, tmp;

	x_dir = dir->x >= 0 ? 1 : -1;
	y_dir = dir->y >= 0 ? 1 : -1;
	z_dir = dir->z >= 0 ? 1 : -1;

	memcpy(&center, &old_root->center, sizeof(center));
	kmVec3Add(&center, &center,
		kmVec3Fill(&tmp, x_dir * half, y_dir * half, z_dir * half));

	oct->root = malloc(sizeof(*oct->root));
	assert(oct->root);

	root_pos = x_dir > 0 ? 1 : 0;
	root_pos += y_dir < 0 ? 4 : 0;
	root_pos += z_dir > 0 ? 2 : 0;

	oct->root->children = calloc(8, sizeof(ne_octree_node));
	assert(oct->root->children);

	for (uint8_t i = 0; i < 8; ++i) {
		if (root_pos == i) {
			memcpy(&oct->root->children[i], old_root,
				sizeof(oct->root->children[i]));
			continue;
		}

		x_dir = i % 2 == 0 ? -1 : 1;
		y_dir = i > 3 ? -1 : 1;
		z_dir = (i < 2 || (i > 3 && i < 6)) ? -1 : 1;

		kmVec3Add(&node_center, &center,
			kmVec3Fill(&tmp, x_dir * half, y_dir * half, z_dir * half));
		_octree_node_init(&oct->root->children[i], &node_center, len,
			oct->min_size, oct->looseness, 20);
	}

	free(old_root);
}

bool
octree_add(
	ne_octree *oct,
	const void *obj)
{
	int i = 0;
	kmVec3 dir;

	while (!_octree_node_add(oct->root, obj)) {
		/* dir = obj->transformed_bounds.center - oct->root->center */
		_octree_grow(oct, &dir );

		if (++i > NOC_GROW_MAX_ITER) {
			//
			return false;
		}
	}

	++oct->count;

	return true;
}

bool
octree_remove(
	ne_octree *oct,
	const void *obj)
{
	if (!_octree_node_remove(oct->root, obj))
		return false;

	--oct->count;
	_octree_node_shrink(oct->root);

	return true;
}

void
octree_get_visible(
	ne_octree *oct,
	const void *frustum,
	rt_array *dst)
{
	_octree_node_get_visible(oct->root, frustum, dst);
}

void
octree_get_coliding(
	ne_octree *oct,
	const void *frustum,
	rt_array *dst)
{
	_octree_node_get_coliding(oct->root, frustum, dst);
}

void
octree_destroy(ne_octree *oct)
{
	if (!oct)
		return;

	_octree_node_destroy(oct->root);
	free(oct);
}

