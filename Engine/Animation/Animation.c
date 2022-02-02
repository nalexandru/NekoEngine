#include <Engine/Resource.h>
#include <Animation/Animation.h>

struct NePipeline *Anim_pipeline;

bool Anim_CreateClip(const char *name, const struct NeAnimationClipCreateInfo *ci, struct NeAnimationClip *ac, NeHandle h);
bool Anim_LoadClip(struct NeResourceLoadInfo *li, const char *args, struct NeAnimationClip *ac, NeHandle h);
void Anim_UnloadClip(struct NeAnimationClip *ac, NeHandle h);

bool
Anim_InitAnimationSystem(void)
{
	if (!E_RegisterResourceType(RES_ANIMATION_CLIP, sizeof(struct NeAnimationClip), (NeResourceCreateProc)Anim_CreateClip,
						(NeResourceLoadProc)Anim_LoadClip, (NeResourceUnloadProc)Anim_UnloadClip))
		return false;
	
	return true;
}

void
Anim_TermAnimationSystem(void)
{

}

