#ifndef NE_TRANSIENT_RESOURCE_H
#define NE_TRANSIENT_RESOURCE_H

/*
 * Transient resource emulation for APIs that don't support them
 */

#include <stdint.h>
#include <Runtime/Runtime.h>

struct NeTransientResource
{
	uint64_t hash;
};

static inline bool
FindTransientResource(struct NeArray *array, uint64_t hash, void *tr, uint64_t size)
{
	size_t id = RT_NOT_FOUND;

	struct NeTransientResource *t;
#ifndef __cplusplus
	Rt_ArrayForEach(t, array) { // The amount of items is small so this is good enough
#else
	Rt_ArrayForEach(t, array, struct NeTransientResource *) { // The amount of items is small so this is good enough
#endif
		if (t->hash != hash)
			continue;

		id = miwa_rtafei;
		break;
	}

	if (id == RT_NOT_FOUND)
		return false;

	memcpy(tr, Rt_ArrayGet(array, id), size);
	Rt_ArrayRemove(array, id);

	return true;
}

#endif /* NE_TRANSIENT_RESOURCE_H */
