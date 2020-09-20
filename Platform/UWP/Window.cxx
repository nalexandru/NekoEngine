#include <stdint.h>

#include <wrl.h>
#include <wrl/client.h>

#include <System/Window.h>

using namespace Platform;
using namespace Windows::UI::ViewManagement;

int
Sys_CreateWindow(void)
{
	return 0;
}

void
Sys_SetWindowTitle(const wchar_t *title)
{
	ApplicationView::GetForCurrentView()->Title = ref new String(title);
}

void
Sys_DestroyWindow(void)
{
	
}
