#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Asset/NMorph.h>

bool
E_LoadNMorphAsset(struct NeStream *stm, struct NeModel *m)
{
	ASSET_INFO;

	ASSET_CHECK_GUARD(NMORPH_1_HEADER);

	struct NeMorph info;
	if (!E_ReadStream(stm, &info, sizeof(info)))
		goto error;

	uint32_t deltaCount;
	if (!E_ReadStream(stm, &deltaCount, sizeof(deltaCount)))
		goto error;
	
	size_t deltaSize = sizeof(struct NeMorphDelta) * deltaCount;
	struct NeMorphDelta *deltas = Sys_Alloc(deltaSize, 1, MH_Asset);
	if (E_ReadStream(stm, deltas, deltaSize) != deltaSize)
		goto error;

	ASSET_CHECK_GUARD(NMORPH_FOOTER);

	uint32_t startInfo = m->morph.count;
	++m->morph.count;
	m->morph.info = Sys_ReAlloc(m->morph.info, sizeof(*m->morph.info), m->morph.count, MH_Asset);
	memcpy(m->morph.info + startInfo, &info, sizeof(info));

	uint32_t startDelta = m->morph.deltaCount;
	m->morph.deltaCount += deltaCount;

	m->morph.deltas = Sys_ReAlloc(m->morph.deltas, sizeof(*m->morph.deltas), m->morph.deltaCount, MH_Asset);
	memcpy(m->morph.deltas + startDelta, deltas, deltaSize);

	return true;

error:

	return false;
}
