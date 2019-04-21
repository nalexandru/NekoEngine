/* Miwa Portable Runtime
 *
 * config.c
 * Author: Alexandru Naiman
 *
 * Configuration System
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (c) 2018-2019, Alexandru Naiman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <system/system.h>
#include <system/compat.h>
#include <system/config.h>
#include <runtime/runtime.h>

static char _config_file[2048];

typedef struct CONFIG_ENTRY
{
	rt_string name;
	rt_variant value;
} config_entry;

static rt_array _config;

uint8_t
sys_config_init(const char *file)
{
	char s[512], s1[512], s2[512];
	int32_t ip[4];
	int32_t i;
	float f;
	FILE *fp = fopen(file, "r");

	if (rt_array_init(&_config, 10, sizeof(config_entry)) != SYS_OK)
		return false;

	if (!fp) {
#ifdef _WIN32	// Windows dosen't set errno to ENOENT
		if (sys_file_exists(file) < 0) {
			rt_array_release(&_config);
			return 2;
		}
#endif
		return errno == ENOENT ? 2 : 1;
	}

	while (fgets(s, 512, fp)) {
		if (s[0] == '#' || s[0] == 0x0 || s[0] == '\n')
			continue;

		if (sscanf(s, "%255s %d.%d.%d.%d", s1,
			&ip[0], &ip[1], &ip[2], &ip[3]) == 5) {
			memset(s2, 0x0, 512);
			snprintf(s2, 512, "%d.%d.%d.%d",
				ip[0], ip[1], ip[2], ip[3]);
			sys_config_set_string(s1, s2);
		} else if (sscanf(s, "%255s %d", s1, &i) == 2) {
			sys_config_set_int(s1, i);
		} else if (sscanf(s, "%255s %f", s1, &f) == 2) {
			sys_config_set_double(s1, f);
		} else if (sscanf(s, "%255s %s", s1, s2) == 2) {
			if (!strncmp(s2, "false", 5))
				sys_config_set_bool(s1, false);
			else if (!strncmp(s2, "true", 4))
				sys_config_set_bool(s1, true);
			else
				sys_config_set_string(s1, s2);
		}

		explicit_bzero(s, 512);
		explicit_bzero(s1, 512);
		explicit_bzero(s2, 512);
	}

	fclose(fp);

	strncpy(_config_file, file, 2047);

	return 0;
}

static INLINE config_entry *
_config_find(const char *key)
{
	uint64_t hash = rt_hash_string(key);
	size_t i = 0;
	config_entry *ent = 0;

	for (i = 0; i < _config.count; ++i) {
		ent = rt_array_get(&_config, i);
		if (ent->name.hash == hash)
			return ent;
	}

	return NULL;
}

void
sys_config_set_int(const char *key,
	int64_t val)
{
	config_entry *ent = _config_find(key);
	config_entry e;

	if (!ent) {
		memset(&e, 0x0, sizeof(e));

		rt_string_init_with_cstr(&e.name, key);
		rt_vt_int(&e.value, val);

		rt_array_add(&_config, &e);
	} else {
		rt_vt_int(&ent->value, val);
	}
}

void
sys_config_set_bool(const char *key,
	bool val)
{
	config_entry *ent = _config_find(key);
	config_entry e;

	if (!ent) {
		memset(&e, 0x0, sizeof(e));

		rt_string_init_with_cstr(&e.name, key);
		rt_vt_bool(&e.value, val);

		rt_array_add(&_config, &e);
	} else {
		rt_vt_bool(&ent->value, val);
	}
}

void
sys_config_set_double(const char *key,
	double val)
{
	config_entry *ent = _config_find(key);
	config_entry e;

	if (!ent) {
		memset(&e, 0x0, sizeof(e));

		rt_string_init_with_cstr(&e.name, key);
		rt_vt_double(&e.value, val);

		rt_array_add(&_config, &e);
	} else {
		rt_vt_double(&ent->value, val);
	}
}

void
sys_config_set_string(const char *key,
	const char *val)
{
	config_entry *ent = _config_find(key);
	config_entry e;

	if (!ent) {
		memset(&e, 0x0, sizeof(e));

		rt_string_init_with_cstr(&e.name, key);
		rt_vt_string(&e.value, val);

		rt_array_add(&_config, &e);
	} else {
		rt_vt_string(&ent->value, val);
	}
}

int64_t
sys_config_get_int(const char *key,
	int64_t def)
{
	config_entry *ent = _config_find(key);

	if (!ent || ent->value.type != RT_VT_INT)
		return def;

	return ent->value.data.i;
}

bool
sys_config_get_bool(const char *key,
	bool def)
{
	config_entry *ent = _config_find(key);

	if (!ent || ent->value.type != RT_VT_BOOL)
		return def;

	return ent->value.data.b;
}

double
sys_config_get_double(const char *key,
	float def)
{
	config_entry *ent = _config_find(key);

	if (!ent || ent->value.type != RT_VT_DOUBLE)
		return def;

	return ent->value.data.d;
}

const char *
sys_config_get_string(const char *key,
	const char *def)
{
	config_entry *ent = _config_find(key);

	if (!ent || ent->value.type != RT_VT_STRING)
		return def;

	return ent->value.data.str.data;
}

bool
sys_config_save(const char *file)
{
	FILE *fp = fopen(file, "w");
	config_entry *ent = 0;
	size_t i;

	if (!fp)
		return false;

	for (i = 0; i < _config.count; ++i) {
		ent = rt_array_get(&_config, i);

		switch (ent->value.type) {
		case RT_VT_STRING:
			fprintf(fp, "%s %s\n", ent->name.data, ent->value.data.str.data);
			break;
		case RT_VT_INT:
			fprintf(fp, "%s %" PRId64 "\n", ent->name.data, ent->value.data.i);
			break;
		case RT_VT_DOUBLE:
			fprintf(fp, "%s %.02f\n", ent->name.data, ent->value.data.d);
			break;
		case RT_VT_BOOL:
			fprintf(fp, "%s %s\n", ent->name.data, ent->value.data.b ? "true" : "false");
			break;
		default:
			// ignore other types
			break;
		}
	}

	fclose(fp);

	return true;
}

uint8_t
sys_config_reload(void)
{
	rt_array_clear(&_config, false);
	return sys_config_init(_config_file);
}

void
sys_config_release(void)
{
	size_t i;
	config_entry *ent = 0;

	for (i = 0; i < _config.count; ++i) {
		ent = rt_array_get(&_config, i);

		rt_string_release(&ent->name);

		if (ent->value.type == RT_VT_STRING)
			free(ent->value.data.str.data);
	}

	rt_array_release(&_config);
}

