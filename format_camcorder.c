#include "ops-linux.h"

static gboolean Format(void) {
  int data;
  
  data=0x0000;
  if(ControlMessageWrite(0xb700,&data,0,LONG_TIMEOUT)== TRUE) {
    Log("format successful. turn camera off and on to see change.");
    return TRUE;
  }
  Log("format failed");
  Log("try unplugging camcorder and starting over");
  return FALSE;
  
}

static gboolean format_camcorder_confirmed( GtkWidget* widget) {
  gboolean success = FALSE;
  if(CheckCameraOpen()==FALSE)
    return FALSE;
  EnableControls(FALSE);
  success = Format();
  EnableControls(TRUE);
  
  return success;

}

gboolean format_camcorder( GtkWidget *widget,
			   GdkEvent *event,
			   gpointer data) {
  

  if(CheckCameraOpen()==FALSE)
    return FALSE;

  return MessageBoxChoice("Do you really want to format the camcorder?\nYou will lose all of your movies.", format_camcorder_confirmed);
  
  
}

