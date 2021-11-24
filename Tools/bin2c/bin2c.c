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
