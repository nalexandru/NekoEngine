#include <Engine/Component.h>

struct Balls 
{
	COMPONENT_BASE;

	EntityHandle entities[100];
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
