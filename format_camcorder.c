#include "ops-linux.h"

static gboolean Format(void) {
  int data;
  
  data=0x0000;
  if(ControlMessageWrite(0xb700, (char*)&data, 0, LONG_TIMEOUT)== TRUE) {
    Log(NOTICE, "format successful. turn camera off and on to see change.");
    return TRUE;
  }
  Log(ERROR, "format failed");
  Log(ERROR, "try unplugging camcorder and starting over");
  return FALSE;
  
}


gboolean format_camcorder( GtkWidget *widget,
			   GdkEvent *event,
			   gpointer data) {
  
  gboolean success;
  if(CheckCameraOpen()==FALSE)
    return FALSE;

  if (main_window) {
    if (MessageBoxConfirm("Do you really want to format the camcorder?\nYou will lose all of your movies.")) {
      EnableControls(FALSE);
      success = Format();
      EnableControls(TRUE);
      return success;
    }
  }
  return FALSE;
}

