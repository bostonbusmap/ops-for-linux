#include "ops-linux.h"

static gboolean MonitorCommand(const char* command) {
  /*  unsigned char buffer[255];
  
  memset(buffer,0,255);
  //Log("strncpy before");
  strncpy(buffer, command, 254);
  //Log("strncpy after");
  if(ControlMessageWrite(0xef00,(int*)buffer,strlen((char*)buffer)+1,LONG_TIMEOUT)== TRUE) {
    Log("monitor commmand succeeded");
    return TRUE;
  }
  Log("monitor command failed");
  //  Log("try unplugging camcorder and starting over");
  return FALSE;*/
  return Monitor(command);
  
}


static gboolean send_monitor_command_confirmed( GtkWidget* data) {
  GtkWidget* textentry = data;
  gboolean success = FALSE;
  //char commandtext[STRINGSIZE];
  char* text;
  //printf("data: %08x\n",data);
  //Log("a");
  text = gtk_entry_get_text(textentry);
  //strncpy(commandtext, text, STRINGSIZE - 1);
  //free(text);
  //Log("b");
  if(CheckCameraOpen()==FALSE)
    return FALSE;
  EnableControls(FALSE);
  success = MonitorCommand(text);
  EnableControls(TRUE);
  
  return success;

}

gboolean send_monitor_command( GtkWidget *widget,
			       GdkEvent *event,
			       gpointer data) {
  

  if(CheckCameraOpen()==FALSE)
    return FALSE;

  return MessageBoxText("WARNING: The command monitor has all kinds of\n\
 commands that will destroy your camera. Be very careful you\n\
 issue the command you mean, and also make sure that you are\n\
 in the correct partition and in the correct directory.\n\
See http://www.maushammer.com/systems/cvscamcorder for a valid\
 command list\n\nCommand to send:",
			send_monitor_command_confirmed);
  
  
}

