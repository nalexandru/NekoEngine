/* NekoEngine
 *
 * octree.h
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

#ifndef _NE_SCENE_OCTREE_H_
#define _NE_SCENE_OCTREE_H_

#include <runtime/runtime.h>

#include <engine/math.h>
#include <engine/status.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ne_octree;
typedef struct ne_octree ne_octree;

ne_octree	*octree_create(kmVec3 *center, float init_size, float looseness, float min_size);

bool		 octree_add(ne_octree *oct, const void *obj);
bool		 octree_remove(ne_octree *oct, const void *obj);

void		 octree_get_visible(ne_octree *oct, const void *frustum, rt_array *dst);
void		 octree_get_coliding(ne_octree *oct, const void *frustum, rt_array *dst);

void		 octree_destroy(ne_octree *oct);

#ifdef __cplusplus
}
#endif

#endif /* _NE_SCENE_OCTREE_H_ */

