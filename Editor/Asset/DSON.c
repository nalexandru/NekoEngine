#include <stdio.h>

#include <Editor/Types.h>
#include <Runtime/Json.h>
#include <System/Memory.h>
#include <System/Log.h>

#include <Editor/Asset/Asset.h>
#include <Editor/Asset/Import.h>

#define DSONIMPMOD	"DSONImporter"

// http://docs.daz3d.com/doku.php/public/dson_spec/start

struct DSONData
{
	jsmntok_t *tokens;
	uint32_t tokenCount;
	char *json;
};

ED_ASSET_IMPORTER(DSON);

static bool _MatchAsset(const char *path);
static bool _ImportAsset(const char *path);

/*static inline void _ConvertMesh(const cgltf_mesh *mesh, const char *name, struct NMesh *nm);
static inline void _SaveMaterial(const cgltf_material *mat, const char *name, const char *path);
static inline void _SaveAnimation(const cgltf_animation *anim, const char *path);
static inline void _SaveImage(const cgltf_image *img, const char *path);*/

static inline uint32_t _parseGeometryLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data);
static inline uint32_t _parseNodeLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data);
static inline uint32_t _parseUVSetLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data);
static inline uint32_t _parseMaterialLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data);
static inline uint32_t _parseModifierLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data);
static inline uint32_t _parseImageLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data);

static bool
_MatchAsset(const char *path)
{
	return strstr(path, ".duf") != NULL;
}

static bool
_ImportAsset(const char *path)
{
	bool rc = false;
	jsmn_parser p;
	jsmn_init(&p);
	struct DSONData data = { 0 };

	FILE *fp = fopen(path, "r");
	if (!fp)
		goto exit;

	fseek(fp, 0, SEEK_END);
	uint32_t jsonSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	data.json = Sys_Alloc(sizeof(*data.json), jsonSize, MH_Asset);

	uint32_t tokenCount = jsmn_parse(&p, data.json, jsonSize, NULL, 0);
	if (tokenCount <= 0)
		goto exit;

	data.tokens = Sys_Alloc(sizeof(*data.tokens), tokenCount, MH_Asset);
	if (!data.tokens)
		goto exit;

	jsmn_init(&p);
	if (jsmn_parse(&p, data.json, jsonSize, data.tokens, tokenCount) < 0)
		goto exit;

	if (data.tokens[0].type != JSMN_OBJECT)
		return false;

	for (uint32_t i = 0; i < tokenCount; ++i) {
		jsmntok_t key = data.tokens[i];
		jsmntok_t val = data.tokens[++i];

		if (JSON_STRING("file_version", key, data.json)) {
			if (!JSON_STRING("0.6.0.0", val, data.json) && !JSON_STRING("0.6.1.0", val, data.json)) {
				Sys_LogEntry(DSONIMPMOD, LOG_CRITICAL, "Unknown DSON version %s", data.json + val.start);
				goto exit;
			}
		} else if (JSON_STRING("asset_info", key, data.json)) {

		} else if (JSON_STRING("geometry_library", key, data.json)) {
			if (val.type != JSMN_ARRAY)
				continue;

			i = _parseGeometryLibrary(i, val.size, &data);
		} else if (JSON_STRING("node_library", key, data.json)) {
			if (val.type != JSMN_ARRAY)
				continue;

			i = _parseNodeLibrary(i, val.size, &data);
		} else if (JSON_STRING("uv_set_library", key, data.json)) {
			if (val.type != JSMN_ARRAY)
				continue;

			i = _parseUVSetLibrary(i, val.size, &data);
		} else if (JSON_STRING("modifier_library", key, data.json)) {
			if (val.type != JSMN_ARRAY)
				continue;

			i = _parseModifierLibrary(i, val.size, &data);
		} else if (JSON_STRING("image_library", key, data.json)) {
			if (val.type != JSMN_ARRAY)
				continue;

			i = _parseImageLibrary(i, val.size, &data);
		} else if (JSON_STRING("material_library", key, data.json)) {
			if (val.type != JSMN_ARRAY)
				continue;

			i = _parseMaterialLibrary(i, val.size, &data);
		} else if (JSON_STRING("scene", key, data.json)) {
			if (val.type != JSMN_OBJECT)
				continue;
		} else {
		}
	}

	rc = true;

exit:
	Sys_Free(data.json);
	Sys_Free(data.tokens);

	return rc;
}

static inline
uint32_t _parseGeometryLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data)
{
	uint32_t pos = startPos;
	for (uint32_t j = 0; j < count; ++j) {
		while (data->tokens[++pos].type != JSMN_OBJECT);

		do {
			const jsmntok_t key = data->tokens[++pos];
			//const jsmntok_t val = data->tokens[++pos];

			if (JSON_STRING("id", key, data->json)) {
			} else if (JSON_STRING("vertices", key, data->json)) {
			} else if (JSON_STRING("polygon_groups", key, data->json)) {
			} else if (JSON_STRING("polygon_material_groups", key, data->json)) {
			} else if (JSON_STRING("polylist", key, data->json)) {
			}
		} while (pos + 1 < data->tokenCount && (data->tokens[pos + 1].type != JSMN_OBJECT));
	}

	return pos;
}

static inline
uint32_t _parseNodeLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data)
{
	uint32_t pos = startPos;
	for (uint32_t j = 0; j < count; ++j) {
		while (data->tokens[++pos].type != JSMN_OBJECT);

		do {
			//const jsmntok_t key = data->tokens[++pos];
			//const jsmntok_t val = data->tokens[++pos];

		} while (pos + 1 < data->tokenCount && (data->tokens[pos + 1].type != JSMN_OBJECT));
	}

	return pos;
}

static inline
uint32_t _parseUVSetLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data)
{
	uint32_t pos = startPos;
	for (uint32_t j = 0; j < count; ++j) {
		while (data->tokens[++pos].type != JSMN_OBJECT);

		do {
			//const jsmntok_t key = data->tokens[++pos];
			//const jsmntok_t val = data->tokens[++pos];

		} while (pos + 1 < data->tokenCount && (data->tokens[pos + 1].type != JSMN_OBJECT));
	}

	return pos;
}

static inline
uint32_t _parseMaterialLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data)
{
	uint32_t pos = startPos;
	for (uint32_t j = 0; j < count; ++j) {
		while (data->tokens[++pos].type != JSMN_OBJECT);

		do {
			//const jsmntok_t key = data->tokens[++pos];
			//const jsmntok_t val = data->tokens[++pos];

		} while (pos + 1 < data->tokenCount && (data->tokens[pos + 1].type != JSMN_OBJECT));
	}

	return pos;
}

static inline
uint32_t _parseModifierLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data)
{
	uint32_t pos = startPos;
	for (uint32_t j = 0; j < count; ++j) {
		while (data->tokens[++pos].type != JSMN_OBJECT);

		do {
			//const jsmntok_t key = data->tokens[++pos];
			//const jsmntok_t val = data->tokens[++pos];

		} while (pos + 1 < data->tokenCount && (data->tokens[pos + 1].type != JSMN_OBJECT));
	}

	return pos;
}

static inline
uint32_t _parseImageLibrary(uint32_t startPos, uint32_t count, const struct DSONData *data)
{
	uint32_t pos = startPos;
	for (uint32_t j = 0; j < count; ++j) {
		while (data->tokens[++pos].type != JSMN_OBJECT);

		do {
			//const jsmntok_t key = data->tokens[++pos];
			//const jsmntok_t val = data->tokens[++pos];

		} while (pos + 1 < data->tokenCount && (data->tokens[pos + 1].type != JSMN_OBJECT));
	}

	return pos;
}
