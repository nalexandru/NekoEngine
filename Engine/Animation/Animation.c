#include <Engine/Resource.h>
#include <Animation/Animation.h>

bool Anim_CreateClip(const char *name, const struct AnimationClipCreateInfo *ci, struct AnimationClip *ac, Handle h);
bool Anim_LoadClip(struct ResourceLoadInfo *li, const char *args, struct AnimationClip *ac, Handle h);
void Anim_UnloadClip(struct AnimationClip *ac, Handle h);

bool
Anim_InitAnimationSystem(void)
{
	if (!E_RegisterResourceType(RES_ANIMATION_CLIP, sizeof(struct AnimationClip), (ResourceCreateProc)Anim_CreateClip,
						(ResourceLoadProc)Anim_LoadClip, (ResourceUnloadProc)Anim_UnloadClip))
		return false;
	
	return true;
}

void
Anim_TermAnimationSystem(void)
{

}

