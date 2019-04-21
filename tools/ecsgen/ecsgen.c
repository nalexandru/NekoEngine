/* NekoEngine Build System
 *
 * ecsgen.c
 * Author: Alexandru Naiman
 *
 * ECS Generator
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <dirent.h>
#include <sys/stat.h>

#ifndef _WIN32
	#include <unistd.h>
	#define MAX_PATH	2048
#elif defined(__MINGW32__)
	#define MAX_PATH	2048
#endif

#define MAX_LINE		4096

#define ECS_COMPONENT		0
#define ECS_SYSTEM		1

#define OP_PARSE		0
#define OP_GEN			1

#define MAX_TYPE_NAME		2048

struct ecs_info
{
	uint8_t type;
	char header[MAX_PATH];
	char args[MAX_LINE];
};

static bool verbose;
static int32_t total_files;
static int32_t total_components;

static inline uint64_t
rt_hash_string(const char *str)
{
	uint64_t hash = 0;

	while (*str) {
		hash += *str++;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

static inline bool
contains_header(uint64_t header,
	uint64_t *headers,
	uint16_t count)
{
	uint16_t i = 0;
	for (i = 0; i < count; ++i)
		if (headers[i] == header)
			return true;
	return false;
}

void
parse(const char *file,
	FILE *fp)
{
	FILE *in_fp = NULL;
	char *ptr = NULL;
	char buff[MAX_LINE];
	struct ecs_info info;
	int32_t count = 0;

	in_fp = fopen(file, "r");
	if (!in_fp)
		return;

	memset(buff, 0x0, sizeof(buff));
	memset(&info, 0x0, sizeof(info));

	snprintf(info.header, MAX_PATH, "%s", file);

	while (fgets(buff, MAX_LINE, in_fp)) {
		ptr = strstr(buff, "NE_REGISTER_COMPONENT");

		if (!ptr || strstr(buff, "#define"))
			continue;

		ptr[strlen(ptr) - 1] = '\0';

		info.type = ECS_COMPONENT;
		snprintf(info.args, MAX_LINE, "%s", ptr + 21);

		if (fwrite(&info, sizeof(info), 1, fp) != 1) {
			fprintf(stderr, "Failed to write output file.");
			exit(-1);
		}

		memset(buff, 0x0, sizeof(buff));
		memset(info.args, 0x0, sizeof(info.args));

		++count;
		++total_components;
	}

	fclose(in_fp);

	if (count)
		++total_files;
}

void
gen(FILE *in_fp,
	FILE *out_fp)
{
	char *ptr = NULL;
	char buff[MAX_LINE];
	uint64_t header = 0;
	uint64_t headers[200];
	uint16_t i = 0, count = 0;

	struct ecs_info info;

	memset(buff, 0x0, sizeof(buff));
	memset(&info, 0x0, sizeof(info));

	fprintf(out_fp, "// THIS IS A GENERATED FILE\n// DO NOT EDIT\n\n");
	fprintf(out_fp, "#include <engine/status.h>\n");
	fprintf(out_fp, "#include <ecs/component.h>\n\n");

	while (fread(&info, sizeof(info), 1, in_fp) == 1) {
		header = rt_hash_string(info.header);

		if (contains_header(header, headers, count))
			continue;

		headers[count++] = header;
		fprintf(out_fp, "#include \"%s\"\n", info.header);
	}

	fseek(in_fp, 0, SEEK_SET);

	fprintf(out_fp, "\n");
	fprintf(out_fp,
		"#define REG_COMP(a, b, c, d) comp_register(a, sizeof(b), c, d)\n\n");
#ifdef _WIN32
	fprintf(out_fp, "__declspec(dllexport) ");
#endif
	fprintf(out_fp, "ne_status\ncomp_sys_register_all(void)\n{\n");
	fprintf(out_fp, "\tne_status ret = NE_FAIL;\n\n");

	while (fread(&info, sizeof(info), 1, in_fp) == 1) {
		fprintf(out_fp,
			"\tif ((ret = REG_COMP%s) != NE_OK)\n\t\treturn ret;\n\n",
			info.args);
	}

	fprintf(out_fp, "\treturn NE_OK;\n}\n");
}

void
parsedir(const char *path,
	FILE *fp)
{
	struct stat st;
	struct dirent *ent = NULL;
	DIR *dir = NULL;
	char full_path[MAX_PATH];

	if ((dir = opendir(path)) == NULL)
		return;

	if (verbose)
		printf("Reading directory %s...\n", path);
	
	while ((ent = readdir(dir))) {
		if (ent->d_name[0] == '.')
			continue;
		
		snprintf(full_path, MAX_PATH, "%s/%s", path, ent->d_name);

		if (stat(full_path, &st) < 0)
			continue;

		if (S_ISDIR(st.st_mode))
			parsedir(full_path, fp);
		else if (S_ISREG(st.st_mode))
			parse(full_path, fp);
	}

	closedir(dir);
}

int
main(int argc,
	char *argv[])
{
	FILE *tmp_fp = NULL, *out_fp = NULL;

	if (argc == 4) {
		if (!strncmp(argv[3], "verbose", strlen(argv[3])))
			verbose = true;
	}

	if (verbose) {
		printf("NekoEngine ECSgen v0.6.0.600\n");
		printf("Copyright (C) 2017-2019 Alexandru Naiman\nAll rights reserved.\n\n");
	}

	tmp_fp = fopen("ecsgen.tmp", "w+");
	if (!tmp_fp) {
		fprintf(stderr, "Cannot open temporary file [%s].\n", "ecsgen.tmp");
		return -1;
	}

	out_fp = fopen(argv[2], "w");
	if (!out_fp) {
		fprintf(stderr, "Cannot open output file [%s].\n", argv[2]);
		fclose(tmp_fp);
		return -1;
	}

	parsedir(argv[1], tmp_fp);
	fseek(tmp_fp, 0, SEEK_SET);
	gen(tmp_fp, out_fp);

	fclose(tmp_fp);
	fclose(out_fp);

	unlink("ecsgen.tmp");

	if (!verbose)
		return 0;

	printf("\nECS generation complete.\n");

	if (total_components == 0)
		printf("No items found.\n");
	else
		printf("Found %d items in %d %s.\n", total_components,
			total_files, total_files > 1 ? "files" : "file");
	
	printf("Output written to %s.\n", argv[2]);

	return 0;
}

