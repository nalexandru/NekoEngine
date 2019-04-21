#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

int useconst = 0;
int zeroterminated = 0;
int usestatic = 0;
int export = 0;
char *name = NULL;

int myfgetc(FILE *f)
{
	int c = fgetc(f);
	if (c == EOF && zeroterminated)
	{
		zeroterminated = 0;
		return 0;
	}
	return c;
}

void process(const char *ifname, const char *ofname)
{
	FILE *ifile, *ofile;
	char buf[PATH_MAX], *p;
	const char *cp;
	int c, col = 1;

	ifile = fopen(ifname, "rb");
	if (ifile == NULL) {
		fprintf(stderr, "cannot open %s for reading\n", ifname);
		exit(1);
	}

	ofile = fopen(ofname, "wb");
	if (ofile == NULL) {
		fprintf(stderr, "cannot open %s for writing\n", ofname);
		exit(1);
	}

	if ((cp = strrchr(ifname, '/')) != NULL) {
		++cp;
	} else {
		if ((cp = strrchr(ifname, '\\')) != NULL)
			++cp;
		else
			cp = ifname;
	}

	strcpy(buf, cp);
	for (p = buf; *p != '\0'; ++p)
		if (!isalnum(*p))
			*p = '_';

	fprintf(ofile, "%s%s%sunsigned char %s[] = {\n",
#if defined(_WIN32) || defined(_WIN64)
			export ? "__declspec(dllexport) " : "",
#else
			"",
#endif
			usestatic ? "static " : "",
			useconst ? "const " : "",
			name ? name : buf);
	
	while ((c = myfgetc(ifile)) != EOF) {
		if (col >= 78 - 6) {
			fputc('\n', ofile);
			col = 1;
		}
		fprintf(ofile, "0x%.2x, ", c);
		col += 6;
	}

	fprintf(ofile, "\n};\n%s%s%sunsigned int %s_size = sizeof(%s);\n",
#if defined(_WIN32) || defined(_WIN64)
			export ? "__declspec(dllexport) " : "",
#else
			"",
#endif
			usestatic ? "static " : "",
			useconst ? "const " : "",
			name ? name : buf,
			name ? name : buf);

	fclose(ifile);
	fclose(ofile);
}

void usage(void)
{
	fprintf(stderr, "usage: bin2c [-cz] <input_file> <output_file>\n");
	exit(1);
}

int main(int argc, char **argv)
{
	if (argc < 3)
		usage();

	while (argc > 3) {
		if (!strcmp(argv[1], "-c")) {
			useconst = 1;
			--argc;
			++argv;
		} else if (!strcmp(argv[1], "-z")) {
			zeroterminated = 1;
			--argc;
			++argv;
		} else if (!strcmp(argv[1], "-s")) {
			usestatic = 1;
			--argc;
			++argv;
		} else if (!strcmp(argv[1], "-e")) {
			export = 1;
			--argc;
			++argv;
		} else if (!strcmp(argv[1], "-n")) {
			name = argv[2];
			argc -= 2;
			argv += 2;
		} else {
			usage();
		}
	}

	process(argv[1], argv[2]);

	return 0;
}
