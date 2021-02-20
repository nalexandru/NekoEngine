#include <stdlib.h>

#include "VulkanDriver.h"

struct Texture *
Vk_CreateTexture(struct RenderDevice *dev, struct TextureCreateInfo *tci)
{
	struct Texture *tex = malloc(sizeof(*tex));
	if (!tex)
		return NULL;

	return tex;
}

const struct TextureDesc *
Vk_TextureDesc(const struct Texture *tex)
{
	return &tex->desc;
}

void
Vk_DestroyTexture(struct RenderDevice *dev, struct Texture *tex)
{
	vkDestroyImageView(dev->dev, tex->imageView, Vkd_allocCb);
	vkDestroyImage(dev->dev, tex->image, Vkd_allocCb);
	free(tex);
}

