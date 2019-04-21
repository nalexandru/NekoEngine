/* NekoEngine
 *
 * asset.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Asset Subsystem
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

#ifndef _NE_ENGINE_ASSET_H_
#define _NE_ENGINE_ASSET_H_

#include <stdint.h>

#include <runtime/runtime.h>

#include <engine/status.h>
#include <graphics/texture.h>

ne_status
asset_load_dds(const uint8_t *data,
	uint64_t data_size,
	uint32_t *width,
	uint32_t *height,
	uint32_t *depth,
	ne_texture_type *type,
	ne_image_format *format,
	uint32_t *levels,
	uint32_t *layers,
	uint8_t **img_data,
	uint64_t *img_data_size,
	bool *should_free);

ne_status
asset_load_png(const uint8_t *data,
	uint64_t data_size,
	uint32_t *width,
	uint32_t *height,
	uint32_t *depth,
	ne_texture_type *type,
	ne_image_format *format,
	uint32_t *levels,
	uint32_t *layers,
	uint8_t **img_data,
	uint64_t *img_data_size,
	bool *should_free);

ne_status
asset_load_tga(const uint8_t *data,
	uint64_t data_size,
	uint32_t *width,
	uint32_t *height,
	uint32_t *depth,
	ne_texture_type *type,
	ne_image_format *format,
	uint32_t *levels,
	uint32_t *layers,
	uint8_t **img_data,
	uint64_t *img_data_size,
	bool *should_free);

ne_status
asset_load_nmesh(const uint8_t *data,
	uint64_t size,
	rt_array *vertices,
	rt_array *indices,
	rt_array *groups);

ne_status
meta_read_floats(const char *str,
	size_t len,
	float *out,
	uint32_t count);

ne_status
meta_read_doubles(const char *str,
	size_t len,
	double *out,
	uint32_t count);

#endif /* _NE_ENGINE_ASSET_H_ */

