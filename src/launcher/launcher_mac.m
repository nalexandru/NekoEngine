/* NekoEngine
 *
 * launcher_mac.m
 * Author: Alexandru Naiman
 *
 * NekoEngine Launcher for Mac OS X systems
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

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <system/config.h>

#include <engine/engine.h>

#define _LAUNCHER_IMPLEMENTATION_
#include "launcher.h"

void
cleanup(void)
{
	engine_destroy();
}

int
launcher_exec(int argc,
	char *argv[])
{
	return 0;
}

int
main(int argc,
	char *argv[])
{
	char c;
	bool show_launcher = false;
	bool wait_rdoc = false;
	int ret;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
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
		if (launcher_exec(argc, argv) != 0) {
			ret = 0;
			goto exit;
		}
	}

	if (wait_rdoc) {
		printf("Press any key after RenderDoc injection\n");
		scanf("%c", &c);
	}

	if (engine_init(argc, argv, 0) != NE_OK) {
		fprintf(stderr, "Initialization failed. The program will now exit.\n");
		ret = -1;
		goto exit;
	}

	ret = engine_run();
    
exit:
	[pool release];
	return ret;
}
