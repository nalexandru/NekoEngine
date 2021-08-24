#ifndef _NE_EDITOR_GUI_MOTIF_INSPECTOR_H_
#define _NE_EDITOR_GUI_MOTIF_INSPECTOR_H_

#include <Engine/Types.h>

void GUI_InspectScene(void);
void GUI_InspectEntity(EntityHandle handle);

bool GUI_InitInspector(int x, int y, int width, int height);
void GUI_ShowInspector(void);
void GUI_TermInspector(void);

#endif /* _NE_EDITOR_GUI_MOTIF_INSPECTOR_H_ */
