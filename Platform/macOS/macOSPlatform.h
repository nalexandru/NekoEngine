#ifndef NE_MACOS_PLATFORM_H
#define NE_MACOS_PLATFORM_H

#ifndef OSX100
#	include <AvailabilityMacros.h>
#endif

#include <Input/Input.h>

extern enum NeButton macOS_keymap[256];

// Apple decided to rename these constants
#if !defined(MAC_OS_X_VERSION_10_12) || MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12

#define NSWindowStyleMaskTitled				NSTitledWindowMask
#define NSWindowStyleMaskClosable			NSClosableWindowMask
#define NSWindowStyleMaskMiniaturizable		NSMiniaturizableWindowMask
#define NSWindowStyleMaskResizable			NSResizableWindowMask

#define NSAlertStyleInformational			NSInformationalAlertStyle
#define NSAlertStyleWarning					NSWarningAlertStyle
#define NSAlertStyleCritical				NSCriticalAlertStyle

#define NSEventMaskAny						NSAnyEventMask
#define NSEventModifierFlagCommand			NSCommandKeyMask
#define NSEventModifierFlagOption			NSAlternateKeyMask
#define NSEventModifierFlagShift			NSShiftKeyMask
#define NSEventModifierFlagControl			NSControlKeyMask
#define NSEventModifierFlagCapsLock			NSAlphaShiftKeyMask

#define convertPointToScreen				convertBaseToScreen

#endif

#if defined(MAC_OS_X_VERSION_10_11) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_11
#	define NE_MACOS_METAL
#endif

#if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
#	define AUTORELEASE_BEGIN	@autoreleasepool {
#	define AUTORELEASE_END		}
#else
#	define AUTORELEASE_BEGIN	{ NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#	define AUTORELEASE_END		[pool release]; }
#endif

#if !defined(MAC_OS_X_VERSION_10_5) || MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_4
	#define kVK_ANSI_A                    0x00
	#define kVK_ANSI_S                    0x01
	#define kVK_ANSI_D                    0x02
	#define kVK_ANSI_F                    0x03
	#define kVK_ANSI_H                    0x04
	#define kVK_ANSI_G                    0x05
	#define kVK_ANSI_Z                    0x06
	#define kVK_ANSI_X                    0x07
	#define kVK_ANSI_C                    0x08
	#define kVK_ANSI_V                    0x09
	#define kVK_ANSI_B                    0x0B
	#define kVK_ANSI_Q                    0x0C
	#define kVK_ANSI_W                    0x0D
	#define kVK_ANSI_E                    0x0E
	#define kVK_ANSI_R                    0x0F
	#define kVK_ANSI_Y                    0x10
	#define kVK_ANSI_T                    0x11
	#define kVK_ANSI_1                    0x12
	#define kVK_ANSI_2                    0x13
	#define kVK_ANSI_3                    0x14
	#define kVK_ANSI_4                    0x15
	#define kVK_ANSI_6                    0x16
	#define kVK_ANSI_5                    0x17
	#define kVK_ANSI_Equal                0x18
	#define kVK_ANSI_9                    0x19
	#define kVK_ANSI_7                    0x1A
	#define kVK_ANSI_Minus                0x1B
	#define kVK_ANSI_8                    0x1C
	#define kVK_ANSI_0                    0x1D
	#define kVK_ANSI_RightBracket         0x1E
	#define kVK_ANSI_O                    0x1F
	#define kVK_ANSI_U                    0x20
	#define kVK_ANSI_LeftBracket          0x21
	#define kVK_ANSI_I                    0x22
	#define kVK_ANSI_P                    0x23
	#define kVK_ANSI_L                    0x25
	#define kVK_ANSI_J                    0x26
	#define kVK_ANSI_Quote                0x27
	#define kVK_ANSI_K                    0x28
	#define kVK_ANSI_Semicolon            0x29
	#define kVK_ANSI_Backslash            0x2A
	#define kVK_ANSI_Comma                0x2B
	#define kVK_ANSI_Slash                0x2C
	#define kVK_ANSI_N                    0x2D
	#define kVK_ANSI_M                    0x2E
	#define kVK_ANSI_Period               0x2F
	#define kVK_ANSI_Grave                0x32
	#define kVK_ANSI_KeypadDecimal        0x41
	#define kVK_ANSI_KeypadMultiply       0x43
	#define kVK_ANSI_KeypadPlus           0x45
	#define kVK_ANSI_KeypadClear          0x47
	#define kVK_ANSI_KeypadDivide         0x4B
	#define kVK_ANSI_KeypadEnter          0x4C
	#define kVK_ANSI_KeypadMinus          0x4E
	#define kVK_ANSI_KeypadEquals         0x51
	#define kVK_ANSI_Keypad0              0x52
	#define kVK_ANSI_Keypad1              0x53
	#define kVK_ANSI_Keypad2              0x54
	#define kVK_ANSI_Keypad3              0x55
	#define kVK_ANSI_Keypad4              0x56
	#define kVK_ANSI_Keypad5              0x57
	#define kVK_ANSI_Keypad6              0x58
	#define kVK_ANSI_Keypad7              0x59
	#define kVK_ANSI_Keypad8              0x5B
	#define kVK_ANSI_Keypad9              0x5C
	#define kVK_Return                    0x24
	#define kVK_Tab                       0x30
	#define kVK_Space                     0x31
	#define kVK_Delete                    0x33
	#define kVK_Escape                    0x35
	#define kVK_Command                   0x37
	#define kVK_Shift                     0x38
	#define kVK_CapsLock                  0x39
	#define kVK_Option                    0x3A
	#define kVK_Control                   0x3B
	#define kVK_RightShift                0x3C
	#define kVK_RightOption               0x3D
	#define kVK_RightControl              0x3E
	#define kVK_Function                  0x3F
	#define kVK_F17                       0x40
	#define kVK_VolumeUp                  0x48
	#define kVK_VolumeDown                0x49
	#define kVK_Mute                      0x4A
	#define kVK_F18                       0x4F
	#define kVK_F19                       0x50
	#define kVK_F20                       0x5A
	#define kVK_F5                        0x60
	#define kVK_F6                        0x61
	#define kVK_F7                        0x62
	#define kVK_F3                        0x63
	#define kVK_F8                        0x64
	#define kVK_F9                        0x65
	#define kVK_F11                       0x67
	#define kVK_F13                       0x69
	#define kVK_F16                       0x6A
	#define kVK_F14                       0x6B
	#define kVK_F10                       0x6D
	#define kVK_F12                       0x6F
	#define kVK_F15                       0x71
	#define kVK_Help                      0x72
	#define kVK_Home                      0x73
	#define kVK_PageUp                    0x74
	#define kVK_ForwardDelete             0x75
	#define kVK_F4                        0x76
	#define kVK_End                       0x77
	#define kVK_F2                        0x78
	#define kVK_PageDown                  0x79
	#define kVK_F1                        0x7A
	#define kVK_LeftArrow                 0x7B
	#define kVK_RightArrow                0x7C
	#define kVK_DownArrow                 0x7D
	#define kVK_UpArrow                   0x7E

	typedef uint32_t NSUInteger;
#endif /* !defined(MAC_OS_X_VERSION_10_5) */

#endif /* NE_MACOS_PLATFORM_H */

/* NekoEngine
 *
 * macOSPlatform.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
