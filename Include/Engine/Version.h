#ifndef _NE_ENGINE_VERSION_H_
#define _NE_ENGINE_VERSION_H_

#ifndef _NESTR
#define _NESTR_INTERNAL(x) #x
#define _NESTR(x) _NESTR_INTERNAL(x)
#endif

#define E_VER_MAJOR		0
#define E_VER_MINOR		8
#define E_VER_BUILD		135
#define E_VER_REVISION	0

#define E_PGM_NAME		L"NekoEngine"
#define E_CODENAME		L"Olivia"
#define E_CPY_STR		L"2015-2021 Alexandru Naiman. All Rights Reserved."

#if E_VER_REVISION == 0
#	define E_VER_STR_A		_NESTR(E_VER_MAJOR) "." _NESTR(E_VER_MINOR) "." _NESTR(E_VER_BUILD)
#else
#	define E_VER_STR_A		_NESTR(E_VER_MAJOR) "." _NESTR(E_VER_MINOR) "." _NESTR(E_VER_BUILD) "." _NESTR(E_VER_REVISION)
#endif

#define E_VER_STR		L""E_VER_STR_A

#endif /* _NE_ENGINE_VERSION_H_ */
