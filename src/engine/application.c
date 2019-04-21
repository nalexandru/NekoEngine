/* NekoEngine
 *
 * application.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Application Interface
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

#include <stddef.h>

#include <system/log.h>
#include <system/config.h>
#include <system/system.h>

#include <engine/application.h>
#include <ecs/ecsys_internal.h>

#define APP_MODULE	"Application"

static void *_app_module_handle;
const ne_app_module *app_module = NULL;
static create_app_module_proc _app_create_proc;

ne_status
app_init(void)
{
	comp_sys_register_all_proc comp_reg_proc = NULL;
	const char *module = NULL;
	ne_status ret = NE_FAIL;

	module = sys_config_get_string("app_module", NULL);
	if (!module) {
		log_entry(APP_MODULE, LOG_INFORMATION,
				"No application module present.");
		return NE_OK;
	}

	_app_module_handle = sys_load_library(module);
	if (!_app_module_handle) {
		ret = NE_LIBRARY_LOAD_FAIL;
		goto error;
	}

	_app_create_proc = (create_app_module_proc)
		sys_get_proc_address(_app_module_handle, "create_app_module");
	if (!_app_create_proc) {
		ret = NE_INVALID_APP_LIB;
		goto error;
	}

	app_module = _app_create_proc();
	if (!app_module) {
		ret = NE_FAIL;
		goto error;
	}

	if (app_module->api_ver != NE_APP_API_VER) {
		ret = NE_API_VERSION_MISMATCH;
		goto error;
	}

	comp_reg_proc = (comp_sys_register_all_proc)
		sys_get_proc_address(_app_module_handle, "comp_sys_register_all");
	if (comp_reg_proc)
		if ((ret = comp_reg_proc()) != NE_OK)
			goto error;

	if ((ret = app_module->init()) != NE_OK)
		goto error;

	return NE_OK;

error:
	app_module = NULL;
	_app_create_proc = NULL;

	if (_app_module_handle)
		sys_unload_library(_app_module_handle);
	_app_module_handle = NULL;

	return ret;
}


void
app_release(void)
{
	if (!app_module)
		return;

	app_module->release();

	app_module = NULL;
	_app_create_proc = NULL;
	sys_unload_library(_app_module_handle);
	_app_module_handle = NULL;
}

