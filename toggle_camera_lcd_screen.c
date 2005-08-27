#include "ops-linux.h"

gboolean toggle_camera_lcd_screen( GtkWidget *widget,
				   GdkEvent *event,
				   gpointer data) {
  if (CheckCameraOpen() == FALSE)
    return FALSE;
  int msg = 0x1f70;
  if (toggle_camera_lcd_screen_is_on == TRUE)
    msg = 0x1f30;
  if(ControlMessageWrite(msg, NULL, 0 ,TIMEOUT)==FALSE) { //SetMode
    Log("Unable to toggle camera lcd screen.");
    return FALSE;
  }
  if (toggle_camera_lcd_screen_is_on == TRUE)
    toggle_camera_lcd_screen_is_on = FALSE;
  else
    toggle_camera_lcd_screen_is_on = TRUE;
  return TRUE;

}
