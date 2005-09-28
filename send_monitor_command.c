#include "ops-linux.h"


gboolean Monitor(const char* command) {
  if(ControlMessageWrite(0xef00, command, strlen(command)+1, LONG_TIMEOUT)==TRUE) {
    Log("monitor command succeeded: ");
    return TRUE;
  }
  Log("monitor command failed");
  return FALSE;
}




gboolean send_monitor_command( GtkWidget *widget,
			       GdkEvent *event,
			       gpointer data) {
  
  //GtkWidget* textentry = data;
  gboolean success = FALSE;
  //char commandtext[STRINGSIZE];
  char* text;
  
  //printf("data: %08x\n",data);
  //Log("a");

  if(CheckCameraOpen()==FALSE)
    return FALSE;

  text = MessageBoxText("WARNING: The command monitor has all kinds of\n\
 commands that will destroy your camera. Be very careful you\n\
 issue the command you mean, and also make sure that you are\n\
 in the correct partition and in the correct directory.\n\
See http://www.maushammer.com/systems/cvscamcorder for a valid\
 command list\n\nCommand to send:");
  if (text == NULL) return FALSE;
  EnableControls(FALSE);
  success = Monitor(text);
  EnableControls(TRUE);
  free(text);
  return success;
}

