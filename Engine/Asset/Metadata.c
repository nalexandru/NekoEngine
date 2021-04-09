#include <stdlib.h>

#include <jsmn.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Runtime/Json.h>

static inline bool _ParseMetadata(struct Metadata *meta);

bool
E_LoadMetadata(struct Metadata *meta, const char *file)
{
	bool rc;
	File f = E_OpenFile(file, IO_READ);
	if (!f)
		return false;
	
	rc = E_LoadMetadataFromFile(meta, f);

	E_CloseFile(f);
	
	return rc;
}

bool
E_LoadMetadataFromFile(struct Metadata *meta, File f)
{
	meta->json = E_ReadFileText(f, &meta->jsonSize, true);
	return _ParseMetadata(meta);
}

bool
E_LoadMetadataFromStream(struct Metadata *meta, struct Stream *stm)
{
	meta->jsonSize = E_StreamLength(stm);
	meta->json = Sys_Alloc(sizeof(*meta->json), (size_t)meta->jsonSize + 1, MH_Transient);

	E_ReadStream(stm, meta->json, meta->jsonSize);

	return _ParseMetadata(meta);
}

void
E_LoadMetadataFloatVector(struct Metadata *meta, struct jsmntok *tok, float *out, uint32_t count)
{
	uint32_t i;
	char *ptr = meta->json + tok->start;
	for (i = 0; i < count; ++i)
		out[i] = strtof(ptr + 2 * (i > 0), &ptr); 
}

static inline bool
_ParseMetadata(struct Metadata *meta)
{
	jsmn_parser p;
	jsmn_init(&p);
	
	if (!meta->json)
		return false;
	
	meta->tokenCount = jsmn_parse(&p, meta->json, (uint32_t)meta->jsonSize, NULL, 0);
	if (meta->tokenCount <= 0)
		return false;
	
	meta->tokens = Sys_Alloc(sizeof(*meta->tokens), meta->tokenCount, MH_Transient);
	if (!meta->tokens)
		return false;
	
	jsmn_init(&p);
	if (jsmn_parse(&p, meta->json, (uint32_t)meta->jsonSize, meta->tokens, meta->tokenCount) < 0)
		return false;
	
	if (meta->tokens[0].type != JSMN_OBJECT)
		return false;
	
	if (!JSON_STRING("id", meta->tokens[1], meta->json))
		return false;
	
	if (!JSON_STRING(meta->id, meta->tokens[2], meta->json))
		return false;
	
	if (!JSON_STRING("version", meta->tokens[3], meta->json))
		return false;
	
	if ((uint32_t)atoi(meta->json + meta->tokens[4].start) != meta->version)
		return false;
	
	meta->tokenCount -= 5;
	meta->tokens += 5;
	
	return true;
}