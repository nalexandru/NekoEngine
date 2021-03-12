#ifndef _E_TYPES_H_
#define _E_TYPES_H_

#include <wchar.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(_WIN32) && !defined(_XBOX) && !defined(RE_BUILTIN)
#	if defined(_ENGINE_INTERNAL_)
#		define ENGINE_API	__declspec(dllexport)
#	else
#		define ENGINE_API	__declspec(dllimport)
#	endif
#else
#	define ENGINE_API
#endif

#undef ALIGN

#if defined (__GNUC__) || defined(__clang__)
#	define ALIGN(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
#	define ALIGN(x) __declspec(align(x))
#else
#	error "Unknown compiler"
#endif

#define E_INVALID_HANDLE (uint64_t)-1

struct Array;
struct Queue;

struct Font;
struct Scene;
struct Model;
struct Stream;
struct Camera;
struct UIContext;
struct AudioClip;
struct AtomicLock;
struct ResourceLoadInfo;
struct ModelCreateInfo;

struct Buffer;
struct BufferDesc;
struct BufferCreateInfo;
struct Texture;
struct TextureDesc;
struct TextureCreateInfo;
struct AccelerationStructure;
struct AccelerationStructureDesc;
struct AccelerationStructureCreateInfo;
struct Shader;
struct Pipeline;
struct PipelineLayout;
struct PipelineLayoutDesc;
struct GraphicsPipelineDesc;
struct Framebuffer;
struct FramebufferDesc;
struct ShaderBindingTable;
struct BlendAttachmentDesc;
struct RenderDevice;
struct RenderDeviceInfo;
struct RenderDeviceProcs;
struct RenderContext;
struct RenderContextProcs;
struct RenderPass;
struct RenderPassDesc;
struct AttachmentDesc;
struct FramebufferAttachmentDesc;
struct DescriptorSet;
struct DescriptorWrite;
struct DescriptorSetLayout;
struct DescriptorSetLayoutDesc;
struct DescriptorBinding;
struct BufferBindInfo;
struct TextureBindInfo;
struct AccelerationStructureBindInfo;
struct Semaphore;

enum ShaderStage;
enum TextureLayout;

typedef void *Swapchain;
typedef void *Surface;

struct Version
{
	uint8_t major;
	uint8_t minor;
	uint8_t build;
	uint8_t revision;
};

enum GPUMemoryFlags;

typedef void *File;
typedef void *Mutex;
typedef void *Futex;
typedef void *Thread;
typedef void *EntityHandle;
typedef void *ConditionVariable;

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

#endif /* _E_TYPES_H_ */
