#define FLAC__NO_DLL

#include <vorbis/vorbisfile.h>
#include <FLAC/stream_decoder.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <System/Log.h>
#include <Audio/Clip.h>

/*******************
*       WAVE       *
********************/

struct RiffHeader
{
	char chunkId[4];
	int chunkSize;
	char format[4];
};

struct WaveFormat
{
	char subChunkId[4];
	int subChunkSize;
	short audioFormat;
	short numChannels;
	int sampleRate;
	int byteRate;
	short blockAlign;
	short bitsPerSample;
};

struct WaveData
{
	char subChunkId[4];
	int subChunk2Size;
};

bool
Asset_LoadWAV(struct NeStream *stm, struct NeAudioClip *clip)
{
	struct WaveFormat waveFormat;
	struct RiffHeader riffHeader;
	struct WaveData waveData;

	E_ReadStream(stm, &riffHeader, sizeof(riffHeader));
	if ((riffHeader.chunkId[0] != 'R' ||
		 riffHeader.chunkId[1] != 'I' ||
		 riffHeader.chunkId[2] != 'F' ||
		 riffHeader.chunkId[3] != 'F') ||
		(riffHeader.format[0] != 'W' ||
		 riffHeader.format[1] != 'A' ||
		 riffHeader.format[2] != 'V' ||
		 riffHeader.format[3] != 'E'))
		return false;

	E_ReadStream(stm, &waveFormat, sizeof(waveFormat));
	if (waveFormat.subChunkId[0] != 'f' ||
		waveFormat.subChunkId[1] != 'm' ||
		waveFormat.subChunkId[2] != 't' ||
		waveFormat.subChunkId[3] != ' ')
		return false;

	//size = Sys_BigEndian() ? Sys_SwapInt32(waveFormat.sub_chunk_size) : waveFormat.sub_chunk_size;

	if (waveFormat.subChunkSize > 16)
		E_SeekStream(stm, sizeof(short), IO_SEEK_CUR);

	E_ReadStream(stm, &waveData, sizeof(waveData));
	if (waveData.subChunkId[0] != 'd' ||
		waveData.subChunkId[1] != 'a' ||
		waveData.subChunkId[2] != 't' ||
		waveData.subChunkId[3] != 'a')
		return false;

#ifdef SYS_BIG_ENDIAN
	const int size = Sys_SwapInt32(waveData.sub_chunk_2_size);
#else
	const int size = waveData.subChunk2Size;
#endif

	clip->data = Sys_Alloc(size, 1, MH_Asset);
	if (!clip->data)
		return false;

	E_ReadStream(stm, clip->data, size);
	clip->byteSize = size;

#ifdef SYS_BIG_ENDIAN
	if (Sys_MachineType() != MT_PS3)
		for (uint32_t i = 0; i < ac->byteSize / 2; ++i)
			ac->data[i] = Sys_SwapUint16(ac->data[i]);
#endif

	clip->sampleRate = waveFormat.sampleRate;
	clip->bitsPerSample = waveFormat.bitsPerSample;
	clip->channels = waveFormat.numChannels;

	return true;
}

/******************
*       OGG       *
*******************/

static size_t OvRead(void *ptr, size_t size, size_t nmemb, void *datasource) { return (size_t)E_ReadStream(datasource, ptr, size * nmemb); }
static int OvSeek(void *datasource, ogg_int64_t offset, int whence) { return (int)E_SeekStream(datasource, offset, whence); }
static int OvClose(void *datasource) { E_CloseStream(datasource); return 0; }
static long OvTell(void *datasource) {	return (long)E_StreamTell(datasource); }

bool
Asset_LoadOGG(struct NeStream *stm, struct NeAudioClip *clip)
{
	ov_callbacks callbacks =
		{
			.read_func = OvRead,
			.seek_func = OvSeek,
			.close_func = OvClose,
			.tell_func = OvTell
		};

	OggVorbis_File file = { 0 };
	if (ov_open_callbacks(stm, &file, NULL, 0, callbacks) < 0)
		return false;

	vorbis_info *info = ov_info(&file, -1);

	clip->sampleRate = (uint32_t)info->rate;
	clip->bitsPerSample = 16;
	clip->channels = info->channels;

	clip->byteSize = (uint32_t)ov_pcm_total(&file, -1) * clip->channels * (clip->bitsPerSample / 8);
	clip->data = Sys_Alloc(clip->byteSize, 1, MH_Asset);
	if (!clip->data)
		return false;

	int bs = -1;
	uint64_t bytesRead = 0;
	do {
#ifdef SYS_BIG_ENDIAN
		bytesRead += ov_read(&file, (char *)clip->data + bytesRead, clip->byteSize - bytesRead, 1, 2, 1, &bs);
#else
		bytesRead += ov_read(&file, (char *)clip->data + bytesRead, (int)(clip->byteSize - bytesRead), 0, 2, 1, &bs);
#endif
	} while (bytesRead < clip->byteSize);

	ov_clear(&file);
	return true;
}

/*******************
*       FLAC       *
********************/

struct FlacData
{
	struct NeStream *stm;
	struct NeAudioClip *clip;
	uint32_t sampleSize;
	uint8_t *ptr;
};

static FLAC__StreamDecoderReadStatus
FlacRead(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, struct FlacData *cd)
{
	const int64_t rd = E_ReadStream(cd->stm, buffer, *bytes);

	if (rd == -1)
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;

	*bytes = (size_t)rd;
	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

static FLAC__StreamDecoderSeekStatus
FlacSeek(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, struct FlacData *cd)
{
	E_SeekStream(cd->stm, absolute_byte_offset, IO_SEEK_SET);
	return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

static FLAC__StreamDecoderTellStatus
FlacTell(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, struct FlacData *cd)
{
	*absolute_byte_offset = E_StreamTell(cd->stm);
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

static FLAC__StreamDecoderLengthStatus
FlacLength(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, struct FlacData *cd)
{
	*stream_length = E_StreamLength(cd->stm);
	return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

static FLAC__bool
FlacEof(const FLAC__StreamDecoder *decoder, struct FlacData *cd)
{
	return E_EndOfStream(cd->stm);
}

static FLAC__StreamDecoderWriteStatus
FlacWrite(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], struct FlacData *cd)
{
	for (uint32_t i = 0; i < frame->header.blocksize; ++i) {
		for (uint32_t j = 0; j < cd->clip->channels; ++j) {
			memcpy(cd->ptr, &buffer[j][i], cd->sampleSize);
			cd->ptr += cd->sampleSize;
		}
	}

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void
FlacMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, struct FlacData *cd)
{
	if (metadata->type != FLAC__METADATA_TYPE_STREAMINFO)
		return;

	cd->clip->channels = metadata->data.stream_info.channels;
	cd->clip->sampleRate = metadata->data.stream_info.sample_rate;
	cd->clip->bitsPerSample = metadata->data.stream_info.bits_per_sample;
	cd->sampleSize = cd->clip->bitsPerSample / 8;

	if (cd->sampleSize % 2)
		++cd->sampleSize;

	cd->clip->byteSize = cd->sampleSize * (uint32_t)metadata->data.stream_info.total_samples * cd->clip->channels;
	cd->clip->data = Sys_Alloc(cd->clip->byteSize, 1, MH_Asset);
	cd->ptr = (uint8_t *)cd->clip->data;
}

static void
FlacError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, struct FlacData *cd)
{
	Sys_LogEntry("FLAC", LOG_CRITICAL, "FLAC decoder error: %s", FLAC__StreamDecoderErrorStatusString[status]);
}

bool
Asset_LoadFLAC(struct NeStream *stm, struct NeAudioClip *clip)
{
	FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
	if (!decoder)
		return false;

	struct FlacData data = { .stm = stm, .clip = clip };
	FLAC__stream_decoder_set_md5_checking(decoder, true);
	FLAC__StreamDecoderInitStatus status = FLAC__stream_decoder_init_stream(decoder,
		(FLAC__StreamDecoderReadCallback)FlacRead,
		(FLAC__StreamDecoderSeekCallback)FlacSeek,
		(FLAC__StreamDecoderTellCallback)FlacTell,
		(FLAC__StreamDecoderLengthCallback)FlacLength,
		(FLAC__StreamDecoderEofCallback)FlacEof,
		(FLAC__StreamDecoderWriteCallback)FlacWrite,
		(FLAC__StreamDecoderMetadataCallback)FlacMetadata,
		(FLAC__StreamDecoderErrorCallback)FlacError,
		&data
	);
	if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
		return false;

	FLAC__stream_decoder_process_until_end_of_stream(decoder);
	FLAC__stream_decoder_delete(decoder);

	return true;
}

/* NekoEngine
 *
 * Audio.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
