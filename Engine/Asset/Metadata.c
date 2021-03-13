#include <stdlib.h>

#include <jsmn.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <System/Memory.h>
#include <Runtime/Json.h>

bool
E_LoadMetadata(struct Metadata *meta, const char *file)
{
	jsmn_parser p;
	jsmn_init(&p);
	
	File f = E_OpenFile(file, IO_READ);
	if (!f)
		return false;
	
	meta->json = E_ReadFileText(f, &meta->jsonSize, true);
	E_CloseFile(f);
	
	if (!meta->json)
		return false;
	
	meta->tokenCount = jsmn_parse(&p, meta->json, meta->jsonSize, NULL, 0);
	if (meta->tokenCount <= 0)
		return false;
	
	meta->tokens = Sys_Alloc(sizeof(*meta->tokens), meta->tokenCount, MH_Transient);
	if (!meta->tokens)
		return false;
	
	jsmn_init(&p);
	if (jsmn_parse(&p, meta->json, meta->jsonSize, meta->tokens, meta->tokenCount) < 0)
		return false;
	
	if (meta->tokens[0].type != JSMN_OBJECT)
		return false;
	
	if (!JSON_STRING("id", meta->tokens[1], meta->json))
		return false;
	
	if (!JSON_STRING(meta->id, meta->tokens[2], meta->json))
		return false;
	
	if (!JSON_STRING("version", meta->tokens[3], meta->json))
		return false;
	
	if (atoi(meta->json + meta->tokens[4].start) != meta->version)
		return false;
	
	meta->tokenCount -= 5;
	meta->tokens += 5;
	
	return true;
}

void
E_LoadMetadataFloatVector(struct Metadata *meta, struct jsmntok *tok, float *out, uint32_t count)
{
	char *ptr = meta->json + tok->start;
	for (uint32_t i = 0; i < count; ++i)
		out[i] = strtof(ptr + (2 * i), &ptr);
}
