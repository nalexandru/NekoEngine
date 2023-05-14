#ifndef NE_INTERFACES_CONSOLE_OUTPUT_H
#define NE_INTERFACES_CONSOLE_OUTPUT_H

#include <Engine/Types.h>

struct NeConsoleOutput
{
	void (*Puts)(const char *text, uint32_t x, uint32_t y);
	uint32_t (*LineHeight)(void);

	bool (*Init)(uint32_t maxLines);
	void (*Term)(void);
};

#define NEIF_CONSOLE_OUTPUT		"NeConsoleOutput"

#endif /* NE_INTERFACES_CONSOLE_OUTPUT_H */
