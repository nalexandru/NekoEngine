#include "MotifGUI.h"
#include "AboutDialog.h"

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/MessageB.h>

#include <Engine/Version.h>

void
GUI_AboutDialog(void)
{
	char messageText[2048];
	snprintf(messageText, sizeof(messageText), "NekoEditor\nVersion: %s \"%s\"\nCopyright (c) %s\nLicensed under BSD 3-clause.", E_VER_STR, E_CODENAME, E_CPY_STR);
	XmString message = XmStringCreateLocalized(messageText);

	Widget dialog = XmCreateInformationDialog(Ed_appShell, "aboutDialog", NULL, 0);
	XtVaSetValues(XtParent(dialog), XmNtitle, "About NekoEditor", NULL);
	XtVaSetValues(dialog, XmNmessageString, message, NULL);

	XmStringFree(message);

	XtManageChild(dialog);
}
