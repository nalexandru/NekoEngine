#include <Engine/Asset.h>
#include <Engine/Resource.h>
#include <Animation/Animation.h>

bool
Anim_CreateClip(const char *name, const struct AnimationClipCreateInfo *ci, struct AnimationClip *ac, Handle h)
{
	memcpy(ac->name, ci->name, sizeof(ac->name));

	ac->ticks = ci->ticks;
	ac->duration = ci->duration;

	if (!Rt_InitArray(&ac->channels, ci->channelCount, sizeof(struct AnimationChannel), MH_Asset))
		return false;

	Rt_FillArray(&ac->channels);

	for (uint32_t i = 0; i < ci->channelCount; ++i) {
		struct AnimationChannel *ch = Rt_ArrayGet(&ac->channels, i);

		ch->hash = Rt_HashString(ci->channels[i].name);
		memcpy(ch->name, ci->channels[i].name, sizeof(ch->name));

		if (!Rt_InitArray(&ch->positionKeys, ci->channels[i].positionCount, sizeof(struct AnimVectorKey), MH_Asset))
			goto error;
		Rt_FillArray(&ch->positionKeys);
		memcpy(ch->positionKeys.data, ci->channels[i].positionKeys, Rt_ArrayByteSize(&ch->positionKeys));

		if (!Rt_InitArray(&ch->rotationKeys, ci->channels[i].rotationCount, sizeof(struct AnimQuatKey), MH_Asset))
			goto error;
		Rt_FillArray(&ch->rotationKeys);
		memcpy(ch->rotationKeys.data, ci->channels[i].rotationKeys, Rt_ArrayByteSize(&ch->rotationKeys));

		if (!Rt_InitArray(&ch->scalingKeys, ci->channels[i].scalingCount, sizeof(struct AnimVectorKey), MH_Asset))
			goto error;
		Rt_FillArray(&ch->scalingKeys);
		memcpy(ch->scalingKeys.data, ci->channels[i].scalingKeys, Rt_ArrayByteSize(&ch->scalingKeys));
	}

	return true;

error:
	for (uint32_t i = 0; i < ci->channelCount; ++i) {
		struct AnimationChannel *ch = Rt_ArrayGet(&ac->channels, i);

		if (!ch->hash)
			continue;

		if (ch->positionKeys.data)
			Rt_TermArray(&ch->positionKeys);
		if (ch->rotationKeys.data)
			Rt_TermArray(&ch->rotationKeys);
		if (ch->scalingKeys.data)
			Rt_TermArray(&ch->scalingKeys);
	}

	Rt_TermArray(&ac->channels);
	
	return false;
}

bool
Anim_LoadClip(struct ResourceLoadInfo *li, const char *args, struct AnimationClip *ac, Handle h)
{
	return E_LoadNAnimAsset(&li->stm, ac);
}

void
Anim_UnloadClip(struct AnimationClip *ac, Handle h)
{
	struct AnimationChannel *ch;
	Rt_ArrayForEach(ch, &ac->channels) {
		Rt_TermArray(&ch->positionKeys);
		Rt_TermArray(&ch->rotationKeys);
		Rt_TermArray(&ch->scalingKeys);
	}
	Rt_TermArray(&ac->channels);
}
