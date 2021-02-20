#include <Audio/Audio.h>
#include <Engine/Resource.h>
#include <Engine/Component.h>
#include <Scene/Components.h>

bool
Au_Init(void)
{
	if (!Au_InitLib())
		return false;

	E_RegisterResourceType(RES_AUDIO_CLIP, sizeof(struct AudioClip) - sizeof(uint8_t) + Au_audioClipDataSize,
		(ResourceCreateProc)Au_CreateClip, (ResourceLoadProc)Au_LoadClip, (ResourceUnloadProc)Au_UnloadClip);

	return E_RegisterComponent(AUDIO_SOURCE_COMP, Au_sourceSize, 1,
		(CompInitProc)Au_InitSourceComponent, (CompTermProc)Au_TermSourceComponent);
}

/*void
Au_Update(void)
{
//	X3DAudioCalculate(_au3DAudio, &_auListener, )
}*/

void
Au_Term(void)
{
	Au_TermLib();
}