#ifndef _RE_SWAPCHAIN_H_
#define _RE_SWAPCHAIN_H_

#include <Render/Types.h>
#include <Render/Device.h>
#include <Render/Texture.h>
#include <Render/Context.h>

extern struct Swapchain *Re_swapchain;

#define RE_INVALID_IMAGE	(void *)UINT64_MAX

static inline struct Swapchain *Re_CreateSwapchain(struct Surface *surface) { return Re_deviceProcs.CreateSwapchain(Re_device, surface); }
static inline void *Re_AcquireNextImage(struct Swapchain *swapchain) { return Re_deviceProcs.AcquireNextImage(Re_device, swapchain); }
static inline bool Re_Present(struct Swapchain *swapchain, void *image) { return Re_deviceProcs.Present(Re_device, Re_CurrentContext(), swapchain, image); }
static inline enum TextureFormat Re_SwapchainFormat(struct Swapchain *swapchain) { return Re_deviceProcs.SwapchainFormat(swapchain); }
static inline struct Texture *Re_SwapchainTexture(struct Swapchain *swapchain, void *image) { return Re_deviceProcs.SwapchainTexture(swapchain, image); }
static inline void Re_DestroySwapchain(struct Swapchain *swapchain) { Re_deviceProcs.DestroySwapchain(Re_device, swapchain); }

#endif /* _RE_SWAPCHAIN_H_ */
