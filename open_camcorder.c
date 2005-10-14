#include "ops-linux.h"
#include <string.h>


gboolean open_camcorder (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  if (Init () == FALSE) {
    Log(DEBUGGING, "Init() == FALSE");	//update log with CCamcorder's's log
    MessageBox ("Couldn't find camcorder");
    return FALSE;
  }
  Log(NOTICE, "Found camcorder.");
  if (Open () == FALSE) {
    Log(DEBUGGING, "Open() == FALSE");	//update log with CCamcorders's log
    Log(ERROR, "Camera can be found but can't be opened.");
    Log(ERROR, "Maybe you're not running as superuser (root)?");
    MessageBox ("Couldn't connect to camcorder\nMaybe you're not running as superuser (root)?");
    return FALSE;
  }
  EnableControls(TRUE);
  return TRUE;
}

