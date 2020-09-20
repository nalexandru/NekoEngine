#ifndef _E_ENGINE_H_
#define _E_ENGINE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void *E_Screen;
extern uint32_t *E_ScreenWidth;
extern uint32_t *E_ScreenHeight;
extern double E_DeltaTime;

bool E_Init(int argc, char *argv[]);
void E_Term(void);

int E_Run(void);
void E_Frame(void);

double E_Time(void);

void E_Shutdown(void);
void E_LoadScene(void);
void E_ActivateScene(void);

#ifdef __cplusplus
}
#endif

#endif /* _E_ENGINE_H_ */