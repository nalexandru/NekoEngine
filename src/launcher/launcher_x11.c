/* NekoEngine
 *
 * launcher_x11.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Launcher for UNIX systems using the X Athena widget set
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2018, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ComboBox.h>
#include <Xm/BulletinB.h>
#include <X11/StringDefs.h>

#include <system/config.h>

#include <engine/engine.h>

#define _LAUNCHER_IMPLEMENTATION_
#include "launcher.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

XtAppContext _launcher_ctx;
static String _launcher_resources[] =
{
	"*bannerLabel.labelString: ",
	"*startButton.labelString: Start",
	"*quitButton.labelString: Quit",
	"*generalButton.labelString: General",
	"*graphicsButton.labelString: Graphics",
	"*audioButton.labelString: Audio",
	"*aboutButton.labelString: About",
	"*scrResLabel.labelString: Resolution:",
	"*gfxDeviceLabel.labelString: Quality:",
	"*sndDeviceLabel.labelString: Device:",
	"*gameNameLabel.labelString: NekoEngine",
	"*gameVersionLabel.labelString: Version: 0.6.0.600",
	"*engineVersionLabel.labelString: Engine Version: 0.6.0.600",
	NULL
};
Widget _tabs[4];
uint8_t _current_tab;

void
_launcher_set_res(Widget w,
	XtPointer client_data,
	XtPointer call_data)
{
	XmComboBoxCallbackStruct *cb = (XmComboBoxCallbackStruct *)call_data;
	
	char *text = (char *)XmStringUnparse(cb->item_or_text, NULL,
		XmMULTIBYTE_TEXT, XmMULTIBYTE_TEXT, NULL, 0, XmOUTPUT_ALL);
	
	char *ptr = strchr(text, 'x');
	*ptr = 0x0;
	++ptr;
	
	sys_config_set_int("width", atoi(text));
	sys_config_set_int("height", atoi(ptr));
	
	XtFree(text);
}

void
_launcher_select_gfx_device(Widget w,
	XtPointer client_data,
	XtPointer call_data)
{
	XmComboBoxCallbackStruct *cb = (XmComboBoxCallbackStruct *)call_data;
	sys_config_set_int("gfx_device_id", cb->item_position);
}

void
_launcher_select_snd_device(Widget w,
	XtPointer client_data,
	XtPointer call_data)
{
	XmComboBoxCallbackStruct *cb = (XmComboBoxCallbackStruct *)call_data;
	sys_config_set_int("snd_device_id", cb->item_position);
}

void
_launcher_set_tab(Widget w,
	XtPointer client_data,
	XtPointer call_data)
{
	size_t tab = (size_t)client_data;

	if (_current_tab == tab)
		return;

	XtUnmanageChild(_tabs[_current_tab]);
	XtManageChild(_tabs[tab]);

	_current_tab = tab;
}

void
_launcher_start(Widget w,
	XtPointer client_data,
	XtPointer call_data)
{
	XtAppSetExitFlag(_launcher_ctx);
}

void
_launcher_quit(Widget w,
	XtPointer client_data,
	XtPointer call_data)
{
	exit(0);
}

void
cleanup()
{
	engine_destroy();
}

int
launcher_exec(int argc,
	char *argv[])
{
	Arg arglist[10];
	Widget w, top, outer;
	int x = 0, y = 0, n = 0;
	char *data = NULL;

	arglist[0].name = XtNwidth;
	arglist[1].name = XtNheight;
	arglist[2].name = XtNx;
	arglist[3].name = XtNy;

	arglist[0].value = LAUNCHER_WIDTH;
	arglist[1].value = LAUNCHER_HEIGHT;

	top = XtAppInitialize(&_launcher_ctx, "NekoEngineLauncher", NULL, 0,
		&argc, argv, _launcher_resources, arglist, 2);
	XtAddCallback(top, XtNdestroyCallback, _launcher_quit, NULL);

	outer = XtCreateManagedWidget("outer", xmBulletinBoardWidgetClass,
		top, arglist, 2);

	arglist[0].value = 75;
	arglist[1].value = 25;
	arglist[2].value = 10;
	arglist[3].value = LAUNCHER_HEIGHT - 35;
	w = XtCreateManagedWidget("quitButton", xmPushButtonWidgetClass, outer,
		arglist, 4);
	XtAddCallback(w, XmNactivateCallback, _launcher_quit, NULL);

	arglist[2].value = LAUNCHER_WIDTH - 85;
	w = XtCreateManagedWidget("startButton", xmPushButtonWidgetClass, outer,
		arglist, 4);
	XtAddCallback(w, XmNactivateCallback, _launcher_start, NULL);

	arglist[2].value = 10;
	arglist[3].value = BANNER_HEIGHT + 20;
	w = XtCreateManagedWidget("generalButton", xmPushButtonWidgetClass,
		outer, arglist, 4);
	XtAddCallback(w, XmNactivateCallback, _launcher_set_tab, (XtPointer)0);

	arglist[2].value += 85;
	w = XtCreateManagedWidget("graphicsButton", xmPushButtonWidgetClass,
		outer, arglist, 4);
	XtAddCallback(w, XmNactivateCallback, _launcher_set_tab, (XtPointer)1);

	arglist[2].value += 85;
	w = XtCreateManagedWidget("audioButton", xmPushButtonWidgetClass,
		outer, arglist, 4);
	XtAddCallback(w, XmNactivateCallback, _launcher_set_tab, (XtPointer)2);

	arglist[2].value += 85;
	w = XtCreateManagedWidget("aboutButton", xmPushButtonWidgetClass,
		outer, arglist, 4);
	XtAddCallback(w, XmNactivateCallback, _launcher_set_tab, (XtPointer)3);

	arglist[0].value = BANNER_WIDTH;
	arglist[1].value = LAUNCHER_HEIGHT - BANNER_HEIGHT - 100;
	arglist[2].value = 10;
	arglist[3].value = BANNER_HEIGHT + 55;
	arglist[4].name = XmNborderWidth;
	arglist[4].value = 1;
	_tabs[0] = XtCreateManagedWidget("generalTab",
		xmBulletinBoardWidgetClass, outer, arglist, 5);
	_tabs[1] = XtCreateManagedWidget("graphicsTab",
		xmBulletinBoardWidgetClass, outer, arglist, 5);
	_tabs[2] = XtCreateManagedWidget("audioTab",
		xmBulletinBoardWidgetClass, outer, arglist, 5);
	_tabs[3] = XtCreateManagedWidget("aboutTab",
		xmBulletinBoardWidgetClass, outer, arglist, 5);

	{ // general tab
		arglist[0].value = 95;
		arglist[1].value = 20;
		arglist[2].value = 10;
		arglist[3].value = 20;
		arglist[4].name = XmNalignment;
		arglist[4].value = XmALIGNMENT_BEGINNING;
		XtCreateManagedWidget("scrResLabel", xmLabelWidgetClass,
			_tabs[0], arglist, 5);

		XmStringTable src_res = (XmStringTable)
			XtMalloc(_launcher_res_count() * sizeof(XmString *));
		
		for (uint32_t i = 0; i < _launcher_res_count(); ++i)
			src_res[i] = XmStringCreateSimple(_launcher_res_str[i]);

		//if (id = (int32_t)Config_GetInt("gfx_device_id", -1); id == -1)
		//	id = 0;

		arglist[0].value = LAUNCHER_WIDTH - 145;
		arglist[2].value = 115;
		arglist[3].value = 10;
		arglist[4].name = XmNcomboBoxType;
		arglist[4].value = XmDROP_DOWN_COMBO_BOX;
		arglist[5].name = XmNitems;
		arglist[5].value = (XtArgVal)src_res;
		arglist[6].name = XmNselectedIndex;
		arglist[6].value = 0;
		
		arglist[1].name = XmNitemCount;
		arglist[1].value = _launcher_res_count();

		w = XtCreateManagedWidget("scrResComboBox",
			xmComboBoxWidgetClass, _tabs[0], arglist, 7);
		XtAddCallback(w, XmNselectionCallback, _launcher_set_res, NULL);
		
		for (uint32_t i = 0; i < _launcher_res_count(); ++i)
			XmStringFree(src_res[i]);
		XtFree((char *)src_res);

		arglist[1].name = XtNheight;
	}

	{ // graphics tab
		arglist[0].value = 40;
		arglist[1].value = 20;
		arglist[2].value = 10;
		arglist[3].value = 20;
		arglist[4].name = XmNalignment;
		arglist[4].value = XmALIGNMENT_BEGINNING;
		XtCreateManagedWidget("gfxDeviceLabel",
			xmLabelWidgetClass, _tabs[1], arglist, 5);

		/*RtArray *devices = NULL;
		if (NeStatus ret = graphics_get_devices(&devices); ret != NE_OK) {
		//	MessageBoxA(HWND_DESKTOP, "Failed to enumerate graphics devices", "FATAL ERROR", MB_OK | MB_ICONERROR);
			exit(-2);
		}

		XmStringTable gfx_devices = (XmStringTable)XtMalloc(rtarray_count(devices) * sizeof(XmString *));
		
		for (uint32_t i = 0; i < rtarray_count(devices); ++i) {
			RtString *str = (RtString *)rtarray_get_ptr(devices, i);
			gfx_devices[i] = XmStringCreateSimple(*(*str));
		}

		int32_t id = 0;
		if (id = (int32_t)config_get_int("gfx_device_id", -1); id == -1)
			id = 0;

		arglist[0].value = LAUNCHER_WIDTH - 90;
		arglist[2].value = 60;
		arglist[3].value = 10;
		arglist[4].name = XmNcomboBoxType;
		arglist[4].value = XmDROP_DOWN_COMBO_BOX;
		arglist[5].name = XmNitems;
		arglist[5].value = (XtArgVal)gfx_devices;
		arglist[6].name = XmNselectedIndex;
		arglist[6].value = id;
		
		arglist[1].name = XmNitemCount;
		arglist[1].value = rtarray_count(devices);

		w = XtCreateManagedWidget("gfxDeviceComboBox", xmComboBoxWidgetClass, _tabs[1], arglist, 7);
		XtAddCallback(w, XmNselectionCallback, _launcher_select_gfx_device, NULL);
		
		for (uint32_t i = 0; i < rtarray_count(devices); ++i) {
			delete (RtString *)rtarray_get_ptr(devices, i);
			XmStringFree(gfx_devices[i]);
		}
		XtFree((char *)gfx_devices);
		
		rtarray_destroy(devices);*/

		arglist[1].name = XtNheight;
	}

	{ // audio tab
		arglist[0].value = 40;
		arglist[1].value = 20;
		arglist[2].value = 10;
		arglist[3].value = 20;
		arglist[4].name = XmNalignment;
		arglist[4].value = XmALIGNMENT_BEGINNING;
		XtCreateManagedWidget("sndDeviceLabel",
			xmLabelWidgetClass, _tabs[2], arglist, 5);

		/*RtArray *devices = NULL;
		if (NeStatus ret = audio_get_devices(&devices); ret != NE_OK) {
		//	MessageBoxA(HWND_DESKTOP, "Failed to enumerate graphics devices", "FATAL ERROR", MB_OK | MB_ICONERROR);
			exit(-2);
		}

		XmStringTable snd_devices = (XmStringTable)XtMalloc(rtarray_count(devices) * sizeof(XmString *));
		
		for (uint32_t i = 0; i < rtarray_count(devices); ++i) {
			RtString *str = (RtString *)rtarray_get_ptr(devices, i);
			snd_devices[i] = XmStringCreateSimple(*(*str));
		}
		
		int32_t id = 0;
		if (id = (int32_t)config_get_int("snd_device_id", -1); id == -1)
			id = 0;

		arglist[0].value = LAUNCHER_WIDTH - 90;
		arglist[2].value = 60;
		arglist[3].value = 10;
		arglist[4].name = XmNcomboBoxType;
		arglist[4].value = XmDROP_DOWN_COMBO_BOX;
		arglist[5].name = XmNitems;
		arglist[5].value = (XtArgVal)snd_devices;
		arglist[6].name = XmNselectedIndex;
		arglist[6].value = id;
		
		arglist[1].name = XmNitemCount;
		arglist[1].value = rtarray_count(devices);

		w = XtCreateManagedWidget("sndDeviceComboBox", xmComboBoxWidgetClass, _tabs[2], arglist, 7);
		XtAddCallback(w, XmNselectionCallback, _launcher_select_snd_device, NULL);

		for (uint32_t i = 0; i < rtarray_count(devices); ++i) {
			delete (RtString *)rtarray_get_ptr(devices, i);
			XmStringFree(snd_devices[i]);
		}
		XtFree((char *)snd_devices);
		
		rtarray_destroy(devices);*/

		arglist[1].name = XtNheight;
	}

	{ // about tab
		arglist[0].value = LAUNCHER_WIDTH - 40;
		arglist[1].value = 20;
		arglist[2].value = 10;
		arglist[3].value = 10;
		arglist[4].name = XmNalignment;
		arglist[4].value = XmALIGNMENT_BEGINNING;
		XtCreateManagedWidget("gameNameLabel", xmLabelWidgetClass,
			_tabs[3], arglist, 5);

		arglist[3].value = 40;
		XtCreateManagedWidget("gameVersionLabel", xmLabelWidgetClass,
			_tabs[3], arglist, 5);

		arglist[3].value = 70;
		XtCreateManagedWidget("engineVersionLabel", xmLabelWidgetClass,
			_tabs[3], arglist, 5);
	}

	arglist[0].value = BANNER_WIDTH;
	arglist[1].value = BANNER_HEIGHT;
	arglist[2].value = 10;
	arglist[3].value = 10;
	w = XtCreateManagedWidget("bannerLabel", xmLabelWidgetClass,
		outer, arglist, 4);

	XtRealizeWidget(top);

	data = (char *)stbi_load("res/banner.png", &x, &y, &n, 4);
	
	if (data) {

		// convert to BGR
		for (int i = 0; i < x * y * n; i += n) {
			uint8_t t = data[i];
			data[i] = data[i + 2];
			data[i + 2] = t;
    		}

		Display *dpy = XtDisplay(w);
		Window win = XtWindow(w);
		XGCValues values;
		int scr = DefaultScreen(dpy);

		GC gc = XCreateGC(dpy, win, 0, &values);
		XSetForeground(dpy, gc, BlackPixel(dpy, scr));
		XSetBackground(dpy, gc, WhitePixel(dpy, scr));
		
		XImage *banner = XCreateImage(dpy, DefaultVisual(dpy, scr),
				DefaultDepth(dpy, scr), ZPixmap, 0, data,
				x, y, 32, x * n);

		if (!banner) {
			printf("Failed to create image\n");
			exit(0);
		}

		XPutImage(dpy, win, gc, banner, 0, 0, 0, 0,
			BANNER_WIDTH, BANNER_HEIGHT);

		XFree(banner);
		stbi_image_free(data);
		
		XSync(dpy, 0);
	}

	XtUnmanageChild(_tabs[1]);
	XtUnmanageChild(_tabs[2]);
	XtUnmanageChild(_tabs[3]);

	XtAppMainLoop(_launcher_ctx);

	Display *dpy = XtDisplay(top);
	XFlush(dpy);
	XSync(dpy, 0);
	
	return 0;
}

int
main(int argc,
	char *argv[])
{
	char c;
	bool show_launcher = false;
	bool wait_rdoc = false;
	
	if (engine_early_init(argc, argv) != NE_OK) {
		fprintf(stderr, "Initialization failed. The program will now exit.\n");
		return -1;
	}

	atexit(cleanup);
	
	for(int i = 0; i < argc; i++) {
		size_t len = strlen(argv[i]);
		if (!strncmp(argv[i], "--launcher", len))
			show_launcher = true;
		else if (!strncmp(argv[i], "--wait-rdoc", len))
			wait_rdoc = true;
	}
	
	if (show_launcher) {
		if (launcher_exec(argc, argv) != 0)
			return 0;
	}

	if (wait_rdoc) {
		printf("Press any key after RenderDoc injection\n");
		scanf("%c", &c);
	}

	if (engine_init(argc, argv, 0) != NE_OK) {
		fprintf(stderr, "Initialization failed. The program will now exit.\n");
		return -1;
	}

	return engine_run();
}

