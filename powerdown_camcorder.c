#include "ops-linux.h"

gboolean PowerdownCamcorder() {
  if(ControlMessageWrite(0x1000,NULL,0, TIMEOUT)==FALSE) { // Request File Read
    Log("failed at 0x10");
    return FALSE;
  }	
  //  return(true);

  return TRUE;

}

gboolean powerdown_camcorder (GtkWidget *widget,
			      GdkEvent *event,
			      gpointer data_mb) {
  if (CheckCameraOpen() == FALSE)
    return FALSE;
  return PowerdownCamcorder();
}
