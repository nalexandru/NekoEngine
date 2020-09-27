#ifndef _E_TYPE_H_
#define _E_TYPE_H_

#include <wchar.h>
#include <stdint.h>
#include <stdbool.h>

#define E_INVALID_HANDLE (uint64_t)-1

struct Font;
struct Scene;
struct Model;
struct Stream;
struct Camera;
struct Texture;
struct UIContext;
struct AudioClip;
struct AtomicLock;
struct ResourceLoadInfo;
struct ModelCreateInfo;
struct TextureCreateInfo;

struct Version
{
	uint8_t major;
	uint8_t minor;
	uint8_t build;
	uint8_t revision;
};

typedef void *File;
typedef void *EntityHandle;
typedef int64_t CompHandle;
typedef size_t CompTypeId;
typedef uint64_t Handle;

typedef void * CommandAllocator;
typedef void * CommandList;

typedef bool (*CompInitProc)(void *, const void **);
typedef void (*CompTermProc)(void *);

typedef void (*ECSysExecProc)(void **comp, void *args);

struct mat3;
struct mat4;

/*#if defined(__PARSER__)
#	define Attribute(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#	define Atribute(...)
#endif*/

#endif /* _E_TYPE_H_ */
