#!/bin/sh

#
# Miwa Configuration Script
# (c) 2018-2019 Alexandru Naiman
#

SRC_ROOT=`dirname $0`/../
TEMPLATE_FILE=$SRC_ROOT/include/system/miwa_config.h.in
OUTPUT_FILE=$SRC_ROOT/include/system/miwa_config.h

test_cc() {
	printf "$1" > /tmp/test.c
	$CC -D__EXTENSIONS__ -o /tmp/test /tmp/test.c $2 > /dev/null 2>&1 
}

gen_config_h() {
	sed -e "s/HAVE_STRCASESTR v/HAVE_STRCASESTR $STRCASESTR/g" 			\
		-e "s/HAVE_STRLCAT v/HAVE_STRLCAT $STRLCAT/g"				\
		-e "s/HAVE_STRLCPY v/HAVE_STRLCPY $STRLCPY/g"				\
		-e "s/HAVE_STRNLEN v/HAVE_STRNLEN $STRNLEN/g"				\
		-e "s/HAVE_SNPRINTF v/HAVE_SNPRINTF $SNPRINTF/g"			\
		-e "s/HAVE_VSNPRINTF v/HAVE_VSNPRINTF $VSNPRINTF/g"			\
		-e "s/HAVE_REALLOCARRAY v/HAVE_REALLOCARRAY $REALLOCARRAY/g"		\
		-e "s/HAVE_EXPLICIT_BZERO v/HAVE_EXPLICIT_BZERO $EXPLICIT_BZERO/g"	\
		-e "s/HAVE_LIBBSD v/HAVE_LIBBSD $LIBBSD/g"				\
		$TEMPLATE_FILE > $OUTPUT_FILE
}

test_cc "void main(){}"
if [ $? -gt 0 ]; then
	echo 'sanity check failed'
fi

#libbsd
LIBBSD=0
test_cc "void main(){}" -lbsd
if [ $? -gt 0 ]; then
	echo 'libbsd not found'
else
	echo 'libbsd found'
	LIBBSD=1
fi

# strcasestr
STRCASESTR=0
test_cc "#define _DEFAULT_SOURCE\n#include <string.h>\nvoid main(){char *a=0, *b=0;strcasestr(a,b);}"
if [ $? -gt 0 ]; then
	echo 'strcasestr not found'
else
	echo 'strcasestr found'
	STRCASESTR=1
fi

# strnlen
STRNLEN=0
test_cc "#define _DEFAULT_SOURCE\n#include <string.h>\nvoid main(){char *a=0;strnlen(a,0);}"
if [ $? -gt 0 ]; then
	echo 'strnlen not found'
else
	echo 'strnlen found'
	STRNLEN=1
fi

# snprintf
SNPRINTF=0
test_cc "#define _DEFAULT_SOURCE\n#include <stdio.h>\nvoid main(){char *a=0, *b=0;snprintf(a,0,\"%s\",b);}"
if [ $? -gt 0 ]; then
	echo 'snprintf not found'
else
	echo 'snprintf found'
	SNPRINTF=1
fi

# vsnprintf
VSNPRINTF=0
test_cc "#define _DEFAULT_SOURCE\n#include <stdio.h>\n#include <stdarg.h>\nvoid main(){char *a=0;va_list va;vsnprintf(a,0,\"%s\",va);}"
if [ $? -gt 0 ]; then
	echo 'vsnprintf not found'
else
	echo 'vsnprintf found'
	VSNPRINTF=1
fi

# strlcat
STRLCAT=0
if [ $LIBBSD -gt 0 ]; then
	test_cc "#define _DEFAULT_SOURCE\n#include <bsd/string.h>\nvoid main(){char *a=0, *b=0;strlcat(a,b,0);}"
else
	test_cc "#define _DEFAULT_SOURCE\n#include <string.h>\nvoid main(){char *a=0, *b=0;strlcat(a,b,0);}"
fi
if [ $? -gt 0 ]; then
	echo 'strlcat not found'
else
	echo 'strlcat found'
	STRLCAT=1
fi

# strlcpy
STRLCPY=0
if [ $LIBBSD -gt 0 ]; then
	test_cc "#define _DEFAULT_SOURCE\n#include <bsd/string.h>\nvoid main(){char *a=0, *b=0;strlcpy(a,b,0);}"
else
	test_cc "#define _DEFAULT_SOURCE\n#include <string.h>\nvoid main(){char *a=0, *b=0;strlcpy(a,b,0);}"
fi
if [ $? -gt 0 ]; then
	echo 'strlcpy not found'
else
	echo 'strlcpy found'
	STRLCPY=1
fi


# reallocarray
REALLOCARRAY=0
if [ $LIBBSD -gt 0 ]; then
	test_cc "#define _DEFAULT_SOURCE\n#include <bsd/stdlib.h>\nvoid main(){char *a=reallocarray(0,1,1);}"
else
	test_cc "#define _DEFAULT_SOURCE\n#include <stdlib.h>\nvoid main(){char *a=reallocarray(0,1,1);}"
fi
if [ $? -gt 0 ]; then
	echo 'reallocarray not found'
else
	echo 'reallocarray found'
	REALLOCARRAY=1
fi

# explicit_bzero
EXPLICIT_BZERO=0
if [ $LIBBSD -gt 0 ]; then
	test_cc "#define _DEFAULT_SOURCE\n#include <bsd/strings.h>\nvoid main(){char *a;explicit_bzero(a,1);}"
else
	test_cc "#define _DEFAULT_SOURCE\n#include <strings.h>\nvoid main(){char *a;explicit_bzero(a,1);}"
fi
if [ $? -gt 0 ]; then
	echo 'explicit_bzero not found'
else
	echo 'explicit_bzero found'
	EXPLICIT_BZERO=1
fi

gen_config_h;

