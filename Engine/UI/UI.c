#include <UI/UI.h>

bool
UI_InitUI(void)
{
	return UI_InitText();
}

void
UI_TermUI(void)
{
	UI_TermText();
}

bool
UI_InitContext(struct UIContext *ctx, const void **args)
{
	uint32_t vertexCount = 64, indexCount = 100, drawCallCount = 10;

	for (args; *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "VertexCount", len))
			vertexCount = atoi((char *)*(++args));
		else if (!strncmp(arg, "IndexCount", len))
			indexCount = atoi((char *)*(++args));
		else if (!strncmp(arg, "DrawCallCount", len))
			drawCallCount = atoi((char *)*(++args));
	}
	
	if (!Rt_InitArray(&ctx->vertices, vertexCount, sizeof(struct UIVertex)))
		return false;

	if (!Rt_InitArray(&ctx->indices, indexCount, sizeof(uint16_t)))
		return false;

	if (!Rt_InitArray(&ctx->draws, drawCallCount, sizeof(struct UIDrawCall)))
		return false;

	return true;
}

void
UI_TermContext(struct UIContext *ctx)
{
	Rt_TermArray(&ctx->vertices);
	Rt_TermArray(&ctx->indices);
	Rt_TermArray(&ctx->draws);
}

void
UI_ResetContext(void **comp, void *args)
{

}

