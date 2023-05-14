#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Asset/NMorph.h>
#include <System/Log.h>

#define NMORPH_MOD	"NMorph"

bool
Asset_LoadMorphPack(struct NeStream *stm, struct NeMorphPack *m)
{
	ASSET_READ_INIT();

	ASSET_CHECK_GUARD(NMORPH_1_HEADER);

	while (!E_EndOfStream(stm)) {
		ASSET_READ_ID();

		if (a.id == NMESH_MORPH_INFO_ID) {
			m->count = a.size / sizeof(*m->morphs);
			m->morphs = Sys_Alloc(a.size, 1, MH_Asset);
			if (!E_ReadStream(stm, m->morphs, a.size))
				goto error;
		} else if (a.id == NMESH_MORPH_DELTA_ID) {
			m->deltaCount = a.size / sizeof(*m->deltas);
			m->deltas = Sys_Alloc(a.size, 1, MH_Asset);
			if (!E_ReadStream(stm, m->deltas, a.size))
				goto error;
		} else if (a.id == NMESH_END_ID) {
			E_SeekStream(stm, -((int64_t)sizeof(a)), IO_SEEK_CUR);
			break;
		} else {
			Sys_LogEntry(NMORPH_MOD, LOG_WARNING, "Unknown section id = 0x%x, size = %d", a.id, a.size);
			E_SeekStream(stm, a.size, IO_SEEK_CUR);
		}

		ASSET_CHECK_GUARD(NMESH_SEC_FOOTER);
	}

	ASSET_CHECK_GUARD(NMORPH_FOOTER);

	return true;

error:
	return false;
}


bool
Asset_LoadMorphPackForModel(struct NeStream *stm, struct NeModel *m)
{
	ASSET_READ_INIT();

	ASSET_CHECK_GUARD(NMORPH_1_HEADER);

	struct NeMorph *info = NULL;
	struct NeMorphDelta *deltas = NULL;
	size_t infoSize = 0, deltaSize = 0;

	while (!E_EndOfStream(stm)) {
		ASSET_READ_ID();

		if (a.id == NMESH_MORPH_INFO_ID) {
			infoSize = a.size;
			info = Sys_Alloc(1, infoSize, MH_Transient);
			if (!E_ReadStream(stm, info, infoSize))
				goto error;
		} else if (a.id == NMESH_MORPH_DELTA_ID) {
			deltaSize = a.size;
			deltas = Sys_Alloc(deltaSize, 1, MH_Asset);
			if (E_ReadStream(stm, deltas, deltaSize) != deltaSize)
				goto error;
		} else if (a.id == NMESH_END_ID) {
			E_SeekStream(stm, -((int64_t)sizeof(a)), IO_SEEK_CUR);
			break;
		} else {
			Sys_LogEntry(NMORPH_MOD, LOG_WARNING, "Unknown section id = 0x%x, size = %d", a.id, a.size);
			E_SeekStream(stm, a.size, IO_SEEK_CUR);
		}

		ASSET_CHECK_GUARD(NMESH_SEC_FOOTER);
	}

	ASSET_CHECK_GUARD(NMORPH_FOOTER);

	uint32_t startInfo = m->morph.count;
	m->morph.count += (uint32_t)(infoSize / sizeof(*info));
	m->morph.info = Sys_ReAlloc(m->morph.info, sizeof(*m->morph.info), m->morph.count, MH_Asset);
	memcpy(m->morph.info + startInfo, info, infoSize);

	uint32_t startDelta = m->morph.deltaCount;
	m->morph.deltaCount += (uint32_t)(deltaSize / sizeof(*deltas));
	m->morph.deltas = Sys_ReAlloc(m->morph.deltas, sizeof(*m->morph.deltas), m->morph.deltaCount, MH_Asset);
	memcpy(m->morph.deltas + startDelta, deltas, deltaSize);

	return true;

error:
	return false;
}
