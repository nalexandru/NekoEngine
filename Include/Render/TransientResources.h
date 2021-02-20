#ifndef _RE_TRANSIENT_RESOURCES_H_
#define _RE_TRANSIENT_RESOURCES_H_

#include <Engine/Types.h>

static inline struct Texture *Re_TransientTexture(const struct TextureCreateInfo *tci) { return NULL; }
static inline struct Buffer *Re_TransientBuffer(const struct BufferCreateInfo *bci) { return NULL; }

#endif /* _RE_TRANSIENT_RESOURCES_H_ */
