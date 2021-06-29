#ifndef _NE_EDITOR_EDITOR_H_
#define _NE_EDITOR_EDITOR_H_

#include <Editor/Types.h>

bool Ed_CreateGUI(void);

#ifndef __APPLE__
void Ed_ProcessCocoaEvents(void);
#endif

#endif /* _NE_EDITOR_EDITOR_H_ */
