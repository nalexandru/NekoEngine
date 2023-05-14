#include <Render/Render.h>
#include <Render/Backend.h>
#include <System/AtomicLock.h>

static uint64_t f_offset;
static struct NeAtomicLock f_lock;

struct NeBuffer *ReP_CreateTransientBuffer(const struct NeBufferDesc *desc, NeBufferHandle location, uint64_t offset, uint64_t *size);

struct NeTexture *
Re_CreateTransientTexture(const struct NeTextureDesc *desc, uint16_t location)
{
	Sys_AtomicLockWrite(&f_lock);

	uint64_t size;
	struct NeTexture *tex = Re_BkCreateTransientTexture(desc, location, f_offset, &size);
	if (tex)
		f_offset += size;

	Sys_AtomicUnlockWrite(&f_lock);

	return tex;
}

struct NeBuffer *
Re_CreateTransientBuffer(const struct NeBufferDesc *desc, NeBufferHandle location)
{
	Sys_AtomicLockWrite(&f_lock);

	uint64_t size;
	struct NeBuffer *buff = ReP_CreateTransientBuffer(desc, location, f_offset, &size);
	if (buff)
		f_offset += size;

	Sys_AtomicUnlockWrite(&f_lock);

	return buff;
}

void
Re_ResetTransientHeap(void)
{
	Sys_AtomicLockWrite(&f_lock);
	f_offset = 0;
	Sys_AtomicUnlockWrite(&f_lock);
}