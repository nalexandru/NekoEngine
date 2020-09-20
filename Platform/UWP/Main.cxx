#include <wrl.h>
#include <wrl/client.h>

#include <ppltasks.h>

#include <Engine/Config.h>
#include <Engine/Engine.h>

#include "EngineView.h"

// NVIDIA Optimus
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
extern "C" _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
extern "C" _declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;

using namespace Claire;

using namespace Windows::UI::Popups;
using namespace Windows::UI::ViewManagement;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Storage;
using namespace Windows::Foundation;

ref class ApplicationSource sealed : IFrameworkViewSource
{
public:
	virtual IFrameworkView ^CreateView()
	{
		return ref new EngineView();
	}
};

[Platform::MTAThread]
int
_RealMain(Platform::Array<Platform::String^>^)
{
	char config_file[_MAX_PATH], log_file[_MAX_PATH],
		dataDir[_MAX_PATH], dir[_MAX_PATH];

	StorageFolder ^sf = ApplicationData::Current->LocalFolder;
	wcstombs(dir, sf->Path->Data(), sizeof(dir));

	snprintf(config_file, _MAX_PATH, "%s\\engine.cfg", dir);
	snprintf(log_file, _MAX_PATH, "%s\\engine.log", dir);

	memset(dir, 0x0, sizeof(dir));
	sf = Package::Current->InstalledLocation;
	wcstombs(dir, sf->Path->Data(), sizeof(dir));

	snprintf(dataDir, _MAX_PATH, "%s\\Data", dir);
	E_SetCVarStr(L"dataDir", dataDir);

/*	if (engine_early_init(7, args) != NE_OK) {
		MessageDialog ^dlg = ref new MessageDialog(L"Initialization failed. The program will now exit.", L"Fatal Error");
		dlg->ShowAsync();

		return -1;
	}*/

	E_SetCVarStr(L"logFile", "D:\\Projects\\Claire\\bin\\EngineUWP.log");

	ApplicationView::PreferredLaunchViewSize = Size(1280, 720);
	ApplicationView::PreferredLaunchWindowingMode = ApplicationViewWindowingMode::PreferredLaunchViewSize;

	ApplicationSource ^src = ref new ApplicationSource();
	CoreApplication::Run(src);

	return 0;
}
