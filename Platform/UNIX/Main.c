#include <Engine/Engine.h>
#include <System/System.h>

int
main(int argc, char *argv[])
{
	if (!E_Init(argc, argv))
		return -1;

	return E_Run();
}
