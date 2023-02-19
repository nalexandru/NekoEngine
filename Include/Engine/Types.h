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
#	define NE_ALIGNED_STRUCT(n, x, y) struct n { y } __attribute__((aligned(x)))
#elif defined(_MSC_VER)
#	define NE_ALIGN(x) __declspec(align(x))
#	define NE_ALIGNED_STRUCT(n, x, y) __declspec(align(x)) struct n { y }
#endif

#ifdef __cplusplus
class ENGINE_API EngineStartupFuncClass
{
public:
	EngineStartupFuncClass(void (*func)(void)) noexcept
	{
		func();
	}
};

#define E_INITIALIZER(x)								\
	static void x(void);								\
	static EngineStartupFuncClass startupFunc ## x(x);	\
	static void x(void)
#else
#if defined (__GNUC__) || defined(__clang__)
#	define E_INITIALIZER(x) \
		static void x(void) __attribute__((constructor)); \
		static void x(void)
#elif defined(_MSC_VER)
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
#	error "You must implement NE_ALIGN and E_INITIALIZER macros in Include/Engine/Types.h for this compiler"
#endif
#endif

#ifndef _SID_COMPILER_
#	define SID(x)	x
#endif

#ifndef __cplusplus
#	define NE_ATOMIC_HDR <stdatomic.h>
#	define NE_ATOMIC_INT volatile atomic_int
#	define NE_ATOMIC_UINT volatile atomic_uint
#else
#	define NE_ATOMIC_HDR <atomic>
#	define NE_ATOMIC_INT volatile std::atomic_int
#	define NE_ATOMIC_UINT volatile std::atomic_uint
#endif

#ifdef _WIN32
#	define ssize_t size_t
#endif

#define E_INVALID_HANDLE	(uint64_t)-1
#define E_HANDLE_TYPE(x)	(uint32_t)((x & (uint64_t)0xFFFFFFFF00000000) >> 32)
#define E_HANDLE_ID(x)		(uint32_t)(x & (uint64_t)0x00000000FFFFFFFF)

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

struct NeEntityComp;
struct NeComponentCreationData;

struct NeVersion
{
	uint8_t major;
	uint8_t minor;
	uint16_t build;
	uint8_t revision;
};

struct NAnim;
struct NMesh;
struct NTexture;
struct NAudio;

typedef uint32_t NeFlags;
typedef uint64_t NeFlags64;

typedef void *NeFile;
typedef void *NeMutex;
typedef void *NeFutex;
typedef void *NeThread;
typedef void *NeEntityHandle;
typedef void *NeConditionVariable;

typedef size_t NeCompTypeId;
typedef uint64_t NeHandle;
typedef NeHandle NeCompHandle;

typedef bool (*NeCompInitProc)(void *, const void **);
typedef void (*NeCompTermProc)(void *);

typedef void (*NeECSysExecProc)(void **comp, void *args);

struct NeVec3;
struct NeVec4;
struct NeMatrix;

/*#if defined(__PARSER__)
#	define Attribute(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#	define Atribute(...)
#endif*/

#endif /* _NE_ENGINE_TYPES_H_ */

/* NekoEngine
 *
 * Types.h
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
