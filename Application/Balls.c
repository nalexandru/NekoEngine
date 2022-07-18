#include <Engine/Component.h>

struct Balls 
{
	NE_COMPONENT_BASE;

	NeEntityHandle entities[100];
};

bool
App_InitBalls(struct Balls *comp, const void **args)
{
	return true;
}

void
App_TermBalls(struct Balls *comp)
{
}
