#ifndef TEST_APPLICATION_H
#define TEST_APPLICATION_H

#ifndef _NESTR
#define _NESTR_INTERNAL(x) #x
#define _NESTR(x) _NESTR_INTERNAL(x)
#endif

#define A_VER_MAJOR		0
#define A_VER_MINOR		8
#define A_VER_BUILD		27
#define A_VER_REVISION	0

#define A_PGM_NAME		L"NekoEngine Test Application"
#define A_CODENAME		L"Olivia"
#define A_CPY_STR		L"2020-2021 Alexandru Naiman. All Rights Reserved."

#if A_VER_REVISION == 0
#	define A_VER_STR_A		_NESTR(A_VER_MAJOR) "." _NESTR(A_VER_MINOR) "." _NESTR(A_VER_BUILD)
#else
#	define A_VER_STR_A		_NESTR(A_VER_MAJOR) "." _NESTR(A_VER_MINOR) "." _NESTR(A_VER_BUILD) "." _NESTR(A_VER_REVISION)
#endif

#define A_VER_STR		L""A_VER_STR_A

#endif /* TEST_APPLICATION_H */
