#include "ops-linux.h"

gboolean powerdown_camcorder (GtkWidget *widget,
			      GdkEvent *event,
			      gpointer data_mb) {
  int data;
  if (CheckCameraOpen() == FALSE)
    return FALSE;
  if(ControlMessageWrite(0x1000,&data,0, TIMEOUT)==FALSE) { // Request File Read
    Log("failed at 0x10");
    return FALSE;
  }	
  //  return(true);


  return TRUE;
}
