#ifndef _ENGINE_PROFILER_H_
#define _ENGINE_PROFILER_H_

void Prof_BeginRegion(const char *name, float r, float g, float b);
void Prof_InsertMarker(const char *name);
void Prof_EndRegion(void);

void Prof_Reset(void);

#endif /* _ENGINE_PROFILER_H_ */
