#include <Engine/IO.h>
#include <Engine/Asset.h>
//#include <Render/Model.h>
#include <Runtime/Array.h>

#define MESH_3_HEADER	"NMESH3 "
#define MESH_FOOTER		"ENDMESH"

#define CHECK_SIZE(x) if (!(x)) return false

#define READ_UINT32(dst, src, pos, size)		\
	CHECK_SIZE(pos + sizeof(uint32_t) < size);		\
	memcpy(&dst, data, sizeof(uint32_t));		\
	if (sys_is_big_endian())			\
		dst = rt_swap_uint32(dst);		\
	data += sizeof(uint32_t);			\
	pos += sizeof(uint32_t);

#define SWAP_VEC2(v)					\
	v.x = rt_swap_float(v.x);			\
	v.y = rt_swap_float(v.y)

#define SWAP_VEC3(v)					\
	v.x = rt_swap_float(v.x);			\
	v.y = rt_swap_float(v.y);			\
	v.z = rt_swap_float(v.z)

bool
E_LoadNMeshAsset(struct Stream *stm, struct Model *m)
{
/*	uint64_t pos = 0;
	char guard[9];
	
	if (pos + sizeof(char) * 7 < dataSize)
		return false;

	memcpy(guard, data, sizeof(char) * sizeof(guard));

	if (!strncmp(guard, MESH_3_HEADER, 7)) {
		pos += sizeof(guard);
		if (!_load_nmesh_3(data, dataSize, pos, m))
			return false;
	}*/

	return false;
}

//static inline bool
//_load_nmesh_3(const uint8_t *data, uint64_t dataSize, uint64_t *posPtr, struct Model *m)
//{
	/*uint64_t pos = *posPtr;
	uint64_t size = 0;
	uint64_t count = 0;

	READ_UINT32(count, data, pos, dataSize);
	m->vertices = calloc(count, sizeof(*m->vertices));

	size = sizeof(struct Vertex) * count;
	CHECK_SIZE(pos + size < dataSize);
	memcpy(m->vertices, data, size);
	data += size; pos += size;*/

//	return false;
//}
