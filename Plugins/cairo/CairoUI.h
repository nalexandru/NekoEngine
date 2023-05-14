#ifndef NE_CAIRO_UI
#define NE_CAIRO_UI

#include <Render/Types.h>

struct NeCairoContext;

#define NE_CAIRO_CONTEXT	"CairoContext"

#ifdef __cplusplus
extern "C" {
#endif

extern NeCompTypeId NE_CAIRO_CONTEXT_ID;

void *CairoUI_Cairo(struct NeCairoContext *ctx);
void CairoUI_Text(struct NeCairoContext *ctx, const char *text, float px, float py, float size, const char *font);

#ifdef __cplusplus
}
#endif

#endif /* NE_CAIRO_UI */