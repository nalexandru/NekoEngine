#ifndef _RE_MEMORY_H_
#define _RE_MEMORY_H_

#include <Engine/Types.h>

enum GPUMemoryType
{
	MT_GPU_LOCAL,
	MT_CPU_READ,
	MT_CPU_WRITE,
	MT_CPU_COHERENT
};

#endif /* _RE_MEMORY_H_ */
