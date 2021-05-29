#ifndef _NE_ENGINE_ENGINE_H_
#define _NE_ENGINE_ENGINE_H_

#include <stdint.h>
#include <stdbool.h>

#include <Engine/Types.h>

ENGINE_API extern void *E_screen;
ENGINE_API extern uint32_t *E_screenWidth;
ENGINE_API extern uint32_t *E_screenHeight;
ENGINE_API extern double E_deltaTime;

bool E_Init(int argc, char *argv[]);

int E_Run(void);
void E_Frame(void);

double E_Time(void);

void E_Shutdown(void);

void E_ScreenResized(uint32_t width, uint32_t height);

void E_Term(void);

#endif /* _NE_ENGINE_ENGINE_H_ */
