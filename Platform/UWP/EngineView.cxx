#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Render/Render.h>

#include "EngineView.h"

#include <ppltasks.h>

using namespace Claire;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Popups;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

using Microsoft::WRL::ComPtr;

bool _UWPWindowVisible = false;
bool _UWPWindowClosed = false;

EngineView::EngineView()
{
#ifdef _DEBUG
	if (!IsDebuggerPresent()) {
		FreeConsole();
		AllocConsole();
		AttachConsole(GetCurrentProcessId());

		(void)freopen("CON", "w", stdout);
		(void)freopen("CON", "w", stderr);

		system("title Project Claire Log Console");
		system("color 0c");
	}
#endif
}

void
EngineView::Initialize(CoreApplicationView ^av)
{
	av->Activated +=
		ref new TypedEventHandler<CoreApplicationView ^, IActivatedEventArgs ^>
			(this, &EngineView::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs ^>(this, &EngineView::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object ^>(this, &EngineView::OnResuming);
}

void
EngineView::SetWindow(CoreWindow ^win)
{
	win->SizeChanged +=
		ref new TypedEventHandler<CoreWindow ^, WindowSizeChangedEventArgs ^>
			(this, &EngineView::OnWindowSizeChanged);

	win->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow ^, VisibilityChangedEventArgs ^>
			(this, &EngineView::OnVisibilityChanged);

	win->Closed +=
		ref new TypedEventHandler<CoreWindow ^, CoreWindowEventArgs ^>
			(this, &EngineView::OnWindowClosed);

	DisplayInformation ^di = DisplayInformation::GetForCurrentView();

	di->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation ^, Object ^>
			(this, &EngineView::OnDpiChanged);

	di->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>
			(this, &EngineView::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>
			(this, &EngineView::OnDisplayContentsInvalidated);

	E_Screen = (void *)win;

	if (!E_ScreenWidth)
		E_ScreenWidth = &E_GetCVarU32(L"screenWidth", (uint32_t)win->Bounds.Width)->u32;
	else
		*E_ScreenWidth = (uint32_t)win->Bounds.Width;

	if (!E_ScreenHeight)
		E_ScreenHeight = &E_GetCVarU32(L"screenHeight", (uint32_t)win->Bounds.Height)->u32;
	else
		*E_ScreenHeight = (uint32_t)win->Bounds.Height;
}

void
EngineView::Load(Platform::String ^entry)
{
	if (!E_Init(0, NULL)) {

		MessageDialog ^md = ref new MessageDialog(L"Initialization failed. The program will now exit.");
		IAsyncOperation<IUICommand ^> ^op = md->ShowAsync();
		
		while (op->Status == AsyncStatus::Started)
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
		
		CoreApplication::Exit();
	}
}

void
EngineView::Run()
{
	E_Run();
}

void
EngineView::Uninitialize()
{
	//
}

void
EngineView::OnActivated(CoreApplicationView^ av, IActivatedEventArgs^ e)
{
	CoreWindow::GetForCurrentThread()->Activate();
}

void
EngineView::OnSuspending(Platform::Object ^sender, SuspendingEventArgs ^args)
{
	SuspendingDeferral ^sd = args->SuspendingOperation->GetDeferral();

	create_task([this, sd]()
	{
		// suspend engine
		sd->Complete();
	});
}

void
EngineView::OnResuming(Platform::Object ^sender, Platform::Object ^args)
{
	//
}

void
EngineView::OnWindowSizeChanged(CoreWindow ^sender, WindowSizeChangedEventArgs ^args)
{
	*E_ScreenWidth = (uint32_t)args->Size.Width;
	*E_ScreenHeight = (uint32_t)args->Size.Height;

	Re_ScreenResized();
}

void
EngineView::OnVisibilityChanged(CoreWindow ^sender, VisibilityChangedEventArgs ^args)
{
	_UWPWindowVisible = args->Visible;
}

void
EngineView::OnWindowClosed(CoreWindow ^sender, CoreWindowEventArgs ^args)
{
	_UWPWindowClosed = true;
}

void
EngineView::OnDpiChanged(DisplayInformation ^sender, Object ^args)
{
	//
}

void
EngineView::OnOrientationChanged(DisplayInformation ^sender, Object ^args)
{
	//
}

void
EngineView::OnDisplayContentsInvalidated(DisplayInformation ^sender, Object ^args)
{
	//
}
