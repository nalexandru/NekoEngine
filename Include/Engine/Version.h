#ifndef _NE_ENGINE_VERSION_H_
#define _NE_ENGINE_VERSION_H_

#ifndef _NESTR
#define _NESTR_INTERNAL(x) #x
#define _NESTR(x) _NESTR_INTERNAL(x)
#endif

#define E_VER_MAJOR		0
#define E_VER_MINOR		8
#define E_VER_BUILD		257
#define E_VER_REVISION	0

#define E_PGM_NAME		"NekoEngine"
#define E_CODENAME		"Olivia"
#define E_CPY_STR		"2015-2022 Alexandru Naiman. All Rights Reserved."

#if E_VER_REVISION == 0
#	define E_VER_STR		_NESTR(E_VER_MAJOR) "." _NESTR(E_VER_MINOR) "." _NESTR(E_VER_BUILD)
#else
#	define E_VER_STR		_NESTR(E_VER_MAJOR) "." _NESTR(E_VER_MINOR) "." _NESTR(E_VER_BUILD) "." _NESTR(E_VER_REVISION)
#endif

#endif /* _NE_ENGINE_VERSION_H_ */
