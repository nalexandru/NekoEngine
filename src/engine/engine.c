/* NekoEngine
 *
 * engine.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Main Source File
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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

#include <stdint.h>

#include <physfs.h>

#include <system/log.h>
#include <system/time.h>
#include <system/system.h>
#include <system/config.h>

#include <gui/gui.h>
#include <ecs/ecsys.h>
#include <scene/scene.h>
#include <scene/camera.h>
#include <sound/sound.h>
#include <engine/io.h>
#include <engine/task.h>
#include <engine/input.h>
#include <engine/engine.h>
#include <engine/window.h>
#include <engine/version.h>
#include <engine/resource.h>
#include <engine/application.h>
#include <graphics/graphics.h>

#define ENGINE_MODULE			"Engine"

bool engine_stop = false;
float engine_draw_delta = 0.0016666;

static bool _early_init_done = false;
static bool _engine_initialized = false;
bool ne_headless = false;
static double _delta_time = 0.0;
static double _start_time = 0.0;
static double _prev_time = 0;
//static double _lag = 0.0;

struct engine_init_func
{
	const char *name;
	ne_status (*init)(void);
};

struct engine_release_func
{
	const char *name;
	void (*release)(void);
};

static struct engine_init_func _init_funcs[] =
{
	{ "Resource Subsystem", res_init },
	{ "Task Subsystem", task_init },
	{ "Scene (Component) Subsystem", comp_sys_init },
	{ "Scene (Entity) Subsystem", entity_sys_init },
	{ "Scene (ECsys) Subsystem", ecsys_init },
	{ "Graphics Subsystem", gfx_init },
	{ "GUI Subsystem", gui_init },
	{ "Sound Subsystem", snd_init },
	{ "Input Subsystem", input_init },
	{ "Scene Subsystem", scn_sys_init },
	{ "Application Module", app_init },
};
#define INIT_FUNC_COUNT	sizeof(_init_funcs) / sizeof(struct engine_init_func)

static struct engine_release_func _release_funcs[] =
{
	{ "Scene Subsystem", scn_sys_release },
	{ "Scene (Component) Subsystem", comp_sys_release },
	{ "Scene (Entity) Subsystem", entity_sys_release },
	{ "Scene (ECsys) Subsystem", ecsys_release },
	{ "Resources", res_unload_all },
	{ "Application Module", app_release },
	{ "Sound Subsystem", snd_release },
	{ "GUI Subsystem", gui_release },
	{ "Graphics Subsystem", gfx_release },
	{ "Input Subsystem", input_release },
	{ "Task Subsystem", task_release },
	{ "Resource Subsystem", res_release },
};
#define RELEASE_FUNC_COUNT	sizeof(_release_funcs) / sizeof(struct engine_release_func)

ne_status
engine_early_init(
	int argc,
	char *argv[])
{
	sys_init("NekoEngine");
	platform_init();
	log_init("engine.log", LOG_ALL);
	sys_config_init("engine.conf");

	_early_init_done = true;

	return NE_OK;
}

ne_status
engine_init(
	int argc,
	char *argv[],
	void *window)
{
	ne_status ret = NE_FAIL;

	if (!_early_init_done)
		if ((ret = engine_early_init(argc, argv)) != NE_OK)
			return ret;

	ne_headless = sys_config_get_bool("headless", false);

	log_entry(ENGINE_MODULE, LOG_INFORMATION,
		"Starting up...");
	log_entry(ENGINE_MODULE, LOG_INFORMATION,
		"NekoEngine v%s", NE_VERSION_STRING);
#ifdef NE_CONFIG_DEBUG
	log_entry(ENGINE_MODULE, LOG_INFORMATION,
		"DEBUG BUILD");
#endif
	log_entry(ENGINE_MODULE, LOG_INFORMATION,
		"Machine: %s", sys_hostname());
	log_entry(ENGINE_MODULE, LOG_INFORMATION,
		"CPU: %s", sys_cpu_name());
	log_entry(ENGINE_MODULE, LOG_INFORMATION,
		"\tFrequency: %d", sys_cpu_freq());
	log_entry(ENGINE_MODULE, LOG_INFORMATION,
		"\tCount: %d", sys_cpu_count());
	log_entry(ENGINE_MODULE, LOG_INFORMATION,
		"\tArchitecture: %s", sys_machine());
	log_entry(ENGINE_MODULE, LOG_INFORMATION,
		"\tBig Endian: %s", sys_is_big_endian() ? "Yes" : "No");
	log_entry(ENGINE_MODULE, LOG_INFORMATION,
		"Platform: %s %s", sys_os_name(), sys_os_version());
	log_entry(ENGINE_MODULE, LOG_INFORMATION,
		"RAM: %.02f GB", (double)sys_mem_total() / 1073741824.0);

	if ((ret = io_sys_init(argv[0])) != NE_OK)
		return ret;

	if (!ne_headless) {
		if (window) {
			ret = window_register(window);
		} else {
			rt_string title;
			rt_string_init_with_format(&title, 100,
				"NekoEngine v%s \"Alma\"", NE_VERSION_STRING);

			ret = window_create(title.data,
				(uint16_t)sys_config_get_int("width", 1280),
				(uint16_t)sys_config_get_int("height", 720));

			rt_string_release(&title);
		}
	}

	if (ret != NE_OK)
		return ret;

	for (uint32_t i = 0; i < INIT_FUNC_COUNT; ++i) {
		log_entry(ENGINE_MODULE, LOG_INFORMATION, "Initializing %s",
				_init_funcs[i].name);

		ret = _init_funcs[i].init();
		if (ret != NE_OK) {
			if (ret != NE_ABORT_START)
				log_entry(ENGINE_MODULE, LOG_CRITICAL,
					"\tFailed to initialize %s: %d",
					_init_funcs[i].name, ret);
			return ret;
		}
	}

	if (sys_config_get_bool("fullscreen", false))
		window_fullscreen((uint16_t)sys_config_get_int("width", 1280),
		(uint16_t)sys_config_get_int("height", 720));

	log_entry(ENGINE_MODULE, LOG_INFORMATION, "Startup complete");

	_start_time = (double)sys_get_time();
	_prev_time = engine_get_time();

	_engine_initialized = true;

	return NE_OK;
}

double
engine_delta_time(void)
{
	return _delta_time;
}

static inline void
_update(void)
{
	input_pre_update();

	if (input_get_key(NE_KEY_END))
		engine_shutdown();

	ecsys_update_group(ECSYS_GROUP_PRE_LOGIC);

	ecsys_update_group(ECSYS_GROUP_LOGIC);
	app_update(_delta_time);

	input_post_update();
}

static inline void
_draw(void)
{
	if (!ne_main_camera) {
		log_entry(ENGINE_MODULE, LOG_WARNING,
			"Will not draw without a camera !");
		return;
	}

	ecsys_update_group(ECSYS_GROUP_PRE_RENDER);

	gfx_draw();
	app_draw();
}

void
engine_frame(void)
{
	double now = engine_get_time();
	_delta_time = now - _prev_time;
	_prev_time = now;
	//_lag += _delta_time;

	//while (_lag >= 0.01666) {
		_update();
	//	_lag -= 0.01666;
	//}

	_draw();

//	static float delta = 0.f;

	//_update();

/*	delta += _delta_time;

	if (delta > engine_draw_delta) {*/

/*		delta = 0.f;
	}*/
}

double
engine_get_time(void)
{
	return ((double)sys_get_time() - _start_time) * 0.000000001;
}

void
engine_screen_resized(
	uint16_t width,
	uint16_t height)
{
	sys_config_set_int("width", width);
	sys_config_set_int("height", height);

	ne_gfx_screen_width = width;
	ne_gfx_screen_height = height;

	gui_screen_resized();

	gfx_screen_resized(width, height);
	app_screen_resized(width, height);

	ecsys_update_single("cam_update_proj_sys");
}

int
engine_run(void)
{
	int ret = 0;

	if (!ne_headless) {
		ret = sys_engine_run();
	} else {
		while (!engine_stop) {
			engine_frame();
			// sleep for n ms until tick rate
		}
	}

	engine_destroy();

	return ret;
}

void
engine_shutdown(void)
{
	engine_stop = true;
}

void
engine_destroy(void)
{
	if (!_engine_initialized)
		return;

	log_entry(ENGINE_MODULE, LOG_INFORMATION, "Shutting down...");

	gfx_wait_idle();

	for (uint32_t i = 0; i < RELEASE_FUNC_COUNT; ++i) {
		log_entry(ENGINE_MODULE, LOG_INFORMATION, "Releasing %s",
				_release_funcs[i].name);

		_release_funcs[i].release();
	}

	io_sys_release();
	platform_release();

	log_entry(ENGINE_MODULE, LOG_INFORMATION, "Shut down complete");

	_engine_initialized = false;
}
