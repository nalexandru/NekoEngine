#include <string.h>

#include <Audio/Clip.h>
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <System/Endian.h>
#include <System/Memory.h>
#include <System/System.h>

struct NeRiffHeader
{
	char chunk_id[4];   ///< Contains the letters "RIFF" in ASCII form (0x52494646 big-endian form)
	int chunk_size;     ///< 36 + SubChunk2Size. This is the size of the rest of the chunk following this number. This is the entire file minus 8 bytes for the two fields not included in this count: ChunkID and ChunkSize.
	char format[4];     ///< Contains the letters "WAVE" (0x57415645 big-endian form).
};

struct NeWaveFormat 
{
	char sub_chunk_id[4];   ///< Contains the letters "fmt " (0x666d7420)
	int sub_chunk_size; ///< 16 for PCM. This is the size of the rest of the Subchunk which follows this number.
	short audio_format; ///< PCM = 1 (i.e. Linear quantization). Values other than 1 indicate some form of compression.
	short num_channels; ///< Mono = 1, Stereo = 2, etc.
	int sample_rate;    ///< 8000, 44100, etc.
	int byte_rate;      ///< == SampleRate * NumChannels * BitsPerSample/8
	short block_align;  ///< == NumChannels + BitsPerSample/8
	short bits_per_sample;  ///< 8 bits = 8, 16 bits = 16, etc.
};

struct NeWaveData
{
	char sub_chunk_id[4];   ///< Contains the letters "data" (0x64617461 big-endian form).
	int sub_chunk_2_size;   ///< == NumSamples * NumChannels * BitsPerSammple/8. This is the number of bytes in the data. You can also think of this as the size of the read of the subchunk following this number.
};

bool
E_LoadWaveAsset(struct NeStream *stm, struct NeAudioClip *ac)
{
	int size;
	struct NeWaveFormat waveFormat;
	struct NeRiffHeader riffHeader;
	struct NeWaveData waveData;

	E_ReadStream(stm, &riffHeader, sizeof(riffHeader));
	if ((riffHeader.chunk_id[0] != 'R' ||
			riffHeader.chunk_id[1] != 'I' ||
			riffHeader.chunk_id[2] != 'F' ||
			riffHeader.chunk_id[3] != 'F') ||
			(riffHeader.format[0] != 'W' ||
				riffHeader.format[1] != 'A' ||
				riffHeader.format[2] != 'V' ||
				riffHeader.format[3] != 'E'))
		return false;

	E_ReadStream(stm, &waveFormat, sizeof(waveFormat));
	if (waveFormat.sub_chunk_id[0] != 'f' ||
			waveFormat.sub_chunk_id[1] != 'm' ||
			waveFormat.sub_chunk_id[2] != 't' ||
			waveFormat.sub_chunk_id[3] != ' ')
		return false;

	size = Sys_BigEndian() ? Sys_SwapInt32(waveFormat.sub_chunk_size) : waveFormat.sub_chunk_size;

	if (waveFormat.sub_chunk_size > 16)
		E_StreamSeek(stm, sizeof(short), IO_SEEK_CUR);

	E_ReadStream(stm, &waveData, sizeof(waveData));
	if (waveData.sub_chunk_id[0] != 'd' ||
			waveData.sub_chunk_id[1] != 'a' ||
			waveData.sub_chunk_id[2] != 't' ||
			waveData.sub_chunk_id[3] != 'a')
		return false;

	size = Sys_BigEndian() ? Sys_SwapInt32(waveData.sub_chunk_2_size) : waveData.sub_chunk_2_size;

	ac->data = Sys_Alloc(size, 1, MH_Asset);
	if (!ac->data)
		return false;

	E_ReadStream(stm, ac->data, size);
	ac->byteSize = size;

	//if (Sys_BigEndian() && Sys_MachineType() != MT_PS3) {
	if (Sys_BigEndian()) {
		uint32_t i;
		for (i = 0; i < ac->byteSize / 2; ++i)
			ac->data[i] = Sys_SwapUint16(ac->data[i]);
	}

	return true;
}
