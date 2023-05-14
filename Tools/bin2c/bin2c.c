#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

int zeroterminated = 0;

static inline void
usage(void)
{
	fprintf(stderr, "usage: bin2c [-cz] <input_file> <output_file>\n");
	exit(1);
}

int
fgetc_t(FILE *f)
{
	int c = fgetc(f);
	if (c == EOF && zeroterminated) {
		zeroterminated = 0;
		return 0;
	}
	return c;
}

int
main(int argc, char **argv)
{
	FILE *ifile = NULL, *ofile = NULL;
	char buf[PATH_MAX], *p = NULL;
	const char *cp = NULL;
	int c = 0, col = 1, useconst = 0;
	
	while (argc > 3) {
		if (!strcmp(argv[1], "-c")) {
			useconst = 1;
			--argc;
			++argv;
		} else if (!strcmp(argv[1], "-z")) {
			zeroterminated = 1;
			--argc;
			++argv;
		} else {
			usage();
		}
	}
	
	if (argc != 3)
		usage();
	
	ifile = fopen(argv[1], "rb");
	if (ifile == NULL) {
		fprintf(stderr, "cannot open %s for reading\n", argv[1]);
		exit(1);
	}
	
	ofile = fopen(argv[2], "wb");
	if (ofile == NULL) {
		fprintf(stderr, "cannot open %s for writing\n", argv[2]);
		exit(1);
	}
	
	if ((cp = strrchr(argv[1], '/')) != NULL) {
		++cp;
	} else {
		if ((cp = strrchr(argv[1], '\\')) != NULL)
			++cp;
		else
			cp = argv[1];
	}
	
	strcpy(buf, cp);
	
	for (p = buf; *p != '\0'; ++p)
		if (!isalnum(*p))
			*p = '_';
	
	fprintf(ofile, "static %sunsigned char %s[] = {\n", useconst ? "const " : "", buf);
	
	while ((c = fgetc_t(ifile)) != EOF) {
		if (col >= 78 - 6) {
			fputc('\n', ofile);
			col = 1;
		}
		
		fprintf(ofile, "0x%.2x, ", c);
		col += 6;
	}
	
	fprintf(ofile, "\n};\n");

	fclose(ifile);
	fclose(ofile);
	
	return 0;
}

/* NekoEngine bin2c
 *
 * bin2c.c
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
