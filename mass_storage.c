#include "ops-linux.h"

gboolean enable_mass_storage(GtkWidget* widget,
			     GdkEvent* event,
			     gpointer data) {
  if (CheckCameraOpen() == FALSE)
    return FALSE;

  if (Monitor("wl 8013dee8 0xa") == TRUE) {
    Log(NOTICE, "Sent Mass Storage enable command");
    MessageBox(
      "Press 'PLAYBACK' on the camcorder to enable Mass Storage mode.\n"
      "\n"
      "Note: You will need to power-cycle the cam and re-open it to restore\n"
      "normal communications with Ops for linux"
    );
    return TRUE;
  } else {
    Log(ERROR, "Failed to send Mass Storage Mode enable command");
  }
  return FALSE;
  
}
