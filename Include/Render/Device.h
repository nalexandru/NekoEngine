#ifndef _RE_DEVICE_H_
#define _RE_DEVICE_H_

#include <stdint.h>
#include <stdbool.h>

#include <Render/Render.h>

#ifdef __cplusplus
extern "C" {
#endif

struct RenderDevice;
struct RenderWorker;

extern struct RenderDevice Re_Device;
extern struct RenderWorker *Re_Workers;

#ifdef __cplusplus
}
#endif

#endif /* _RE_DEVICE_H_ */