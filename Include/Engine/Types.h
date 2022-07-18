#ifndef _NE_ENGINE_TYPES_H_
#define _NE_ENGINE_TYPES_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(_WIN32) && !defined(_XBOX) && !defined(RE_BUILTIN)
#	if defined(_ENGINE_INTERNAL_)
#		define ENGINE_API	__declspec(dllexport)
#		define ENGINE_IMP	__declspec(dllimport)
#	else
#		define ENGINE_API	__declspec(dllimport)
#		define APP_API		__declspec(dllexport)
#	endif
#else
#	define ENGINE_API
#endif

#if defined (__GNUC__) || defined(__clang__)
#	define NE_ALIGN(x) __attribute__((aligned(x)))

#	define E_INITIALIZER(x) \
		static void x(void) __attribute__((constructor)); \
		static void x(void)

#elif defined(_MSC_VER)
#	define NE_ALIGN(x) __declspec(align(x))

#	pragma section(".CRT$XCU", read)
#	define _INIT(f, p) \
		static void f(void); \
		__declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
		__pragma(comment(linker, "/include:" p #f "_")) \
		static void f(void)

#	ifdef _WIN64
#		define E_INITIALIZER(x)	_INIT(x, "")
#	else
#		define E_INITIALIZER(x) _INIT(x, "_")
#	endif
#else
#	error "Unknown compiler"
#endif

#ifndef _SID_COMPILER_
#	define SID(x)	x
#endif

#define E_INVALID_HANDLE (uint64_t)-1

struct NeArray;
struct NeQueue;

struct NeFont;
struct NeScene;
struct NeLight;
struct NeStream;
struct NeCamera;
struct NeFrustum;
struct NeUIContext;
struct NeAudioClip;
struct NeAtomicLock;
struct NeResourceLoadInfo;

struct NeAudioClip;
struct NeAudioClipCreateInfo;
struct NeAudioSource;
struct NeAudioListener;

struct NeBone;
struct NeSkeleton;
struct NeSkeletonNode;
struct NeAnimationClip;
struct NeAnimationClipCreateInfo;

struct NeComponentCreationData;

struct NeVersion
{
	uint8_t major;
	uint8_t minor;
	uint8_t build;
	uint8_t revision;
};

typedef void *NeFile;
typedef void *NeMutex;
typedef void *NeFutex;
typedef void *NeThread;
typedef void *NeEntityHandle;
typedef void *NeConditionVariable;

typedef int64_t NeCompHandle;
typedef size_t NeCompTypeId;
typedef uint64_t NeHandle;

typedef bool (*NeCompInitProc)(void *, const void **);
typedef void (*NeCompTermProc)(void *);

typedef void (*NeECSysExecProc)(void **comp, void *args);

struct NeMat3;
struct NeMatrix;

/*#if defined(__PARSER__)
#	define Attribute(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#	define Atribute(...)
#endif*/

#endif /* _NE_ENGINE_TYPES_H_ */
