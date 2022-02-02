#ifndef _NE_EDITOR_GUI_H_
#define _NE_EDITOR_GUI_H_

void Ed_ShowProjectDialog(void);

bool Ed_CreateGUI(void);

void EdGUI_ProcessEvents(void);

void EdGUI_MessageBox(const char *title, const char *message);

void EdGUI_ShowProgressDialog(const char *text);
void EdGUI_UpdateProgressDialog(const char *text);
void EdGUI_HideProgressDialog(void);

void Ed_TermGUI(void);

#endif /* _NE_EDITOR_GUI_H_ */
