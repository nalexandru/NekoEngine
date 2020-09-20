#include <wrl.h>

int _RealMain(Platform::Array<Platform::String ^> ^);

[Platform::MTAThread]
int main(Platform::Array<Platform::String ^> ^args)
{
	return _RealMain(args);
}
