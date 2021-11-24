#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <System/Endian.h>
#include <System/Memory.h>
#include <Render/Render.h>

// DDS info:
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb943991(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb943984(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb943983(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb173059(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb172411(v=vs.85).aspx
 
#define DDS_MAGIC           0x20534444
 
// surface flags
#define DDSD_CAPS           0x1
#define DDSD_HEIGHT         0x2
#define DDSD_WIDTH          0x4
#define DDSD_PITCH          0x8
#define DDSD_PIXELFORMAT    0x1000
#define DDSD_MIPMAPCOUNT    0x20000
#define DDSD_LINEARSIZE     0x80000
#define DDSD_DEPTH          0x800000
 
// pixel format flags
#define DDPF_ALPHAPIXELS    0x1
#define DDPF_ALPHA          0x2
#define DDPF_FOURCC         0x4
#define DDPF_RGB            0x40
#define DDPF_RGBA           0x41
#define DDPF_YUV            0x200
#define DDPF_LUMINANCE      0x20000
 
// dwCaps
#define DDSCAPS_COMPLEX     0x8
#define DDSCAPS_MIPMAP      0x400000
#define DDSCAPS_TEXTURE     0x1000
 
// dwCaps2
#define DDSCAPS2_CUBEMAP                0x200
#define DDSCAPS2_CUBEMAP_POSITTIVEX     0x400
#define DDSCAPS2_CUBEMAP_NEGATIVEX      0x800
#define DDSCAPS2_CUBEMAP_POSITTIVEY     0x1000
#define DDSCAPS2_CUBEMAP_NEGATIVEY      0x2000
#define DDSCAPS2_CUBEMAP_POSITTIVEZ     0x8000
#define DDSCAPS2_CUBEMAP_ALLFACES       0xFC00
#define DDSCAPS2_VOLUME                 0x200000
 
// compressed texture types
#define FOURCC_DXT1 0x31545844L // MAKEFOURCC('D','X','T','1')
#define FOURCC_DXT2 0x32545844L // MAKEFOURCC('D','X','T','2')
#define FOURCC_DXT3 0x33545844L // MAKEFOURCC('D','X','T','3')
#define FOURCC_DXT4 0x34545844L // MAKEFOURCC('D','X','T','4')
#define FOURCC_DXT5 0x35545844L // MAKEFOURCC('D','X','T','5')
#define FOURCC_DX10 0x30315844L // MAKEFOURCC('D','X','1','0')

#define DXGI_FORMAT_BC1_UNORM			71
#define DXGI_FORMAT_BC2_UNORM			74
#define DXGI_FORMAT_BC3_UNORM			77
#define DXGI_FORMAT_BC4_UNORM			80
#define DXGI_FORMAT_BC4_SNORM			81
#define DXGI_FORMAT_BC5_UNORM			83
#define DXGI_FORMAT_BC5_SNORM			84
#define DXGI_FORMAT_BC6H_TYPELESS		94
#define DXGI_FORMAT_BC6H_UF16			95
#define DXGI_FORMAT_BC6H_SF16			96
#define DXGI_FORMAT_BC7_TYPELESS		97
#define DXGI_FORMAT_BC7_UNORM			98
#define DXGI_FORMAT_BC7_UNORM_SRGB		99

typedef struct
{
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwFourCC;
	uint32_t dwRGBBitCount;
	uint32_t dwRBitMask;
	uint32_t dwGBitMask;
	uint32_t dwBBitMask;
	uint32_t dwABitMask;
} DDS_PIXELFORMAT;
 
struct DDS_HEADER
{
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwHeight;
	uint32_t dwWidth;
	uint32_t dwPitchOrLinearSize;
	uint32_t dwDepth;
	uint32_t dwMipMapCount;
	uint32_t dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t dwCaps;
	uint32_t dwCaps2;
	uint32_t dwCaps3;
	uint32_t dwCaps4;
	uint32_t dwReserved2;
};
 
struct DDS_HEADER_DXT10
{
	uint32_t dxgiFormat;
	uint32_t resourceDimension;
	uint32_t miscFlag;
	uint32_t arraySize;
	uint32_t miscFlags2;
};

static inline void _swap(uint32_t *p, uint32_t count);
static inline void _readCompressedFormat(const struct DDS_HEADER *hdr, const struct DDS_HEADER_DXT10 *hdrDXT10, struct TextureCreateInfo *tci);

bool
E_LoadDDSAsset(struct Stream *stm, struct TextureCreateInfo *tci)
{
	void *data = NULL;
	uint32_t magic = 0;
	struct DDS_HEADER hdr = { 0 };
	struct DDS_HEADER_DXT10 hdrDXT10 = { 0 };
	
	E_ReadStream(stm, &magic, sizeof(magic));
	if (Sys_BigEndian())
		magic = Sys_SwapUint32(magic);
		
	if (magic != DDS_MAGIC)
		return false;
		
	E_ReadStream(stm, &hdr, sizeof(hdr));
	
	if (hdr.ddspf.dwFlags & DDPF_FOURCC && hdr.ddspf.dwFourCC == FOURCC_DX10)
		E_ReadStream(stm, &hdrDXT10, sizeof(hdrDXT10));
	
	if (Sys_BigEndian()) {
		_swap((uint32_t *)&hdr, sizeof(hdr) / sizeof(uint32_t));
		_swap((uint32_t *)&hdrDXT10, sizeof(hdrDXT10) / sizeof(uint32_t));
	}
	
	// validate header
	if (hdr.dwSize != sizeof(hdr))
		return false;
		
	if (hdr.ddspf.dwFlags & DDPF_FOURCC)
		_readCompressedFormat(&hdr, &hdrDXT10, tci);
	else
		return false;

	tci->desc.type = hdr.dwCaps2 & DDSCAPS2_CUBEMAP ? TT_Cube : TT_2D;
	
	// all ok
	tci->dataSize = (uint32_t)stm->size - (uint32_t)stm->pos;
	data = Sys_Alloc(tci->dataSize, 1, MH_Asset);
	E_ReadStream(stm, data, tci->dataSize);

	tci->desc.width = hdr.dwWidth;
	tci->desc.height = hdr.dwHeight;
	tci->desc.depth = hdr.dwDepth ? hdr.dwDepth : 1;
	tci->desc.mipLevels = hdr.dwMipMapCount;
	tci->desc.arrayLayers = hdr.dwCaps2 & DDSCAPS2_CUBEMAP ? 6 : 1;
	tci->data = data;

	return true;
}

static inline void
_swap(uint32_t *p, uint32_t count)
{
	uint32_t i;
	for (i = 0; i < count; ++i)
		p[i] = Sys_SwapUint32(p[i]);
}

static inline void
_readCompressedFormat(const struct DDS_HEADER *hdr,
	const struct DDS_HEADER_DXT10 *hdrDXT10, struct TextureCreateInfo *tci)
{
	switch (hdr->ddspf.dwFourCC) {
	/*case FOURCC_DXT1: tci->desc.format = TF_BC; break;
	case FOURCC_DXT2: tci->desc.format = TF_DXT2; break;
	case FOURCC_DXT3: tci->desc.format = TF_DXT3; break;
	case FOURCC_DXT4: tci->desc.format = TF_DXT4; break;
	case FOURCC_DXT5: tci->desc.format = TF_DXT5; break;*/
	case FOURCC_DX10: {
		switch (hdrDXT10->dxgiFormat) {
		//case DXGI_FORMAT_BC1_UNORM: tci->desc.format = TF_BC5_SNORM; break;
		//case DXGI_FORMAT_BC2_UNORM: tci->format = TF_BC2; break;
		//case DXGI_FORMAT_BC3_UNORM: tci->format = TF_BC3; break;
		//case DXGI_FORMAT_BC4_UNORM:
		//case DXGI_FORMAT_BC4_SNORM: tci->format = TF_BC4; break;
		case DXGI_FORMAT_BC5_UNORM: tci->desc.format = TF_BC5_UNORM; break;
		case DXGI_FORMAT_BC5_SNORM: tci->desc.format = TF_BC5_SNORM; break;
		case DXGI_FORMAT_BC6H_UF16: tci->desc.format = TF_BC6H_UF16; break;
		case DXGI_FORMAT_BC6H_SF16: tci->desc.format = TF_BC6H_SF16; break;
		case DXGI_FORMAT_BC7_UNORM: tci->desc.format = TF_BC7_UNORM; break;
		case DXGI_FORMAT_BC7_UNORM_SRGB: tci->desc.format = TF_BC7_SRGB; break;
		}
	}
	}
}
