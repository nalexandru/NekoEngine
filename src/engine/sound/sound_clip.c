/* NekoEngine
 *
 * audio_clip.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Audio Clip
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>

#include <physfs.h>
#include <vorbis/vorbisfile.h>

#include <system/log.h>
#include <system/compat.h>
#include <sound/sound_defs.h>
#include <sound/sound_clip.h>

#define AL_BUFFER_SIZE		65536
#define DATA_SIZE			524288
#define SOUND_CLIP_MODULE	"OpenAL_SoundClip"

size_t
_ov_cb_read(
	void *ptr,
	size_t size,
	size_t nmemb,
	void *ds)
{
	return PHYSFS_readBytes(ds, ptr, size * nmemb);
}

int
_ov_cb_seek(
	void *ds,
	ogg_int64_t offset,
	int whence)
{
	ogg_int64_t dst = offset;

	if (whence == SEEK_CUR)
		dst += PHYSFS_tell(ds);
	else if (whence == SEEK_END)
		dst = PHYSFS_fileLength(ds) - offset;

	return PHYSFS_seek(ds, dst);
}

int
_ov_cb_close(void *ds)
{
	return PHYSFS_close(ds);
}

long
_ov_cb_tell(void *ds)
{
	return (long)PHYSFS_tell(ds);
}

static inline int
_load_ogg(
	const char *path,
	ALenum *format,
	uint8_t **data,
	ALsizei *size,
	ALsizei *freq)
{
	int bit_stream = 0;
	long bytes = 0;
	char *buff = NULL;
	long data_size = DATA_SIZE;
	long data_used = 0;
	PHYSFS_File *fp = NULL;
	vorbis_info *info = NULL;
	OggVorbis_File ogg_file;
	ov_callbacks cb;
	uint8_t *tmp = NULL;

	cb.read_func = _ov_cb_read;
	cb.seek_func = _ov_cb_seek;
	cb.close_func = _ov_cb_close;
	cb.tell_func = _ov_cb_tell;

	fp = PHYSFS_openRead(path);
	if (!fp)
		return NE_OPEN_READ_FAIL;

	if (ov_open_callbacks(fp, &ogg_file, NULL, 0, cb) < 0)
		return NE_IO_FAIL;

	info = ov_info(&ogg_file, -1);
	*format = info->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	*freq = info->rate;
	*data = reallocarray(NULL, data_size, sizeof(uint8_t));

	buff = calloc(AL_BUFFER_SIZE, sizeof(char));
	if (!buff)
		return NE_FAIL;

	do {
		memset(buff, 0x0, AL_BUFFER_SIZE);
		bytes = ov_read(&ogg_file, buff, AL_BUFFER_SIZE, 0, 2, 1, &bit_stream);

		if (data_used + bytes >= data_size) {
			tmp = reallocarray(*data, (size_t)data_size + DATA_SIZE, sizeof(uint8_t));
			if (tmp == NULL) {
				free(buff);
				free(*data);
				return NE_ALLOC_FAIL;
			}

			*data = tmp;
			data_size += DATA_SIZE;
		}

		memcpy(*data + data_used, buff, bytes);
		data_used += bytes;
	} while (bytes > 0);

	ov_clear(&ogg_file);

	*size = (int)data_used;

	PHYSFS_close(fp);
	free(buff);

	return NE_OK;
}

ne_sound_clip *
snd_clip_load(const char *file)
{
	ALsizei size = 0, freq = 0;
	ALenum format = 0;
	ALvoid *data = NULL;
	ne_sound_clip *clip = NULL;

	clip = calloc(1, sizeof(*clip));
	if (!clip)
		return NULL;

	if (_load_ogg(file, &format, (uint8_t **)&data, &size, &freq) != NE_OK) {
		log_entry(SOUND_CLIP_MODULE, LOG_DEBUG, "Failed to load clip from [%s]",
					file);
		free(clip);
		return NULL;
	}

	alGenBuffers(1, &clip->buffer);
	alBufferData(clip->buffer, format, data, size, freq);

	free(data);

	log_entry(SOUND_CLIP_MODULE, LOG_DEBUG, "Loaded clip from [%s]", file);

	return clip;
}

void
snd_clip_destroy(ne_sound_clip *clip)
{
	if (!clip)
		return;

	alDeleteBuffers(1, &clip->buffer);
	free(clip);
}

