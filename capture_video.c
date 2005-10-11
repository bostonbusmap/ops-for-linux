#include "ops-linux.h"

gboolean CaptureVideo(const char* filename) {
  char buffer[BUFSIZE];
  int count = 0, x = 0, round = 0;
  u32 fourbytes = 0x00000000;
  u16 twobytes = 0x0000;
  FILE* file = NULL;
  
  //  int msg = 0x1f70;
  if (ControlMessageWrite(0x1f30, NULL, 0, TIMEOUT) == FALSE) {
    Log(ERROR, "failure on 0x1f30");
    return FALSE;
  }
  fourbytes = 0;

  if (ControlMessageRead(0xd000, (char*)&fourbytes, 4, TIMEOUT) == FALSE) {
    Log(ERROR, "failure on 0xd000");
    fourbytes = le32_to_cpu(fourbytes);
    Log(ERROR, "0xd000 returned: %02x %02x %02x %02x",fourbytes & 0xff, (fourbytes & 0xff00) >> 8, (fourbytes & 0xff0000) >> 16, (fourbytes & 0xff000000) >> 24);
    return FALSE;
  }
  Log(NOTICE, "0xd000 returned: %02x %02x %02x %02x",fourbytes & 0xff, (fourbytes & 0xff00) >> 8, (fourbytes & 0xff0000) >> 16, (fourbytes & 0xff000000) >> 24);
  

  if (ControlMessageWrite(0xf002, NULL, 0, TIMEOUT) == FALSE) {
    Log(ERROR, "failure on 0xf002");
    return FALSE;
  }
  if (ControlMessageWrite(0x2000, NULL, 0, TIMEOUT) == FALSE) {
    Log(ERROR,"failure on 0x2000");
    return FALSE;
  }
  if (ControlMessageWrite(0x2f0f, NULL, 0, TIMEOUT) == FALSE) {
    Log(ERROR,"failure on 0x2f0f");
    return FALSE;
  }
  if (ControlMessageWrite(0x2650, NULL, 0, TIMEOUT) == FALSE) {
    Log(ERROR,"failure on 0x2650");
    return FALSE;
  }
  if (ControlMessageWrite(0xe107, NULL, 0, TIMEOUT) == FALSE) {
    Log(ERROR,"failure on 0xe107");
    return FALSE;
  }
  twobytes = 0xffff;
  if (ControlMessageWrite(0x2502, (char*)&twobytes, 2, TIMEOUT) == FALSE) {
    Log(ERROR,"failure on 0x2502");
    return FALSE;
  }
  
  if (ControlMessageWrite(0x1f70, NULL, 0, TIMEOUT) == FALSE) {
    Log(ERROR,"failure on 0x1f70");
    return FALSE;
  }
  fourbytes = 0;

  if (ControlMessageRead(0xd000, (char*)&fourbytes, 0, TIMEOUT) == FALSE) {
    Log(ERROR,"failure on 0xd000");
    fourbytes = le32_to_cpu(fourbytes);
    Log(ERROR,"0xd000 returned: %02x %02x %02x %02x",fourbytes & 0xff, (fourbytes & 0xff00) >> 8, (fourbytes & 0xff0000) >> 16, (fourbytes & 0xff000000) >> 24);
    return FALSE;
  }
  Log(NOTICE, "0xd000 returned: %02x %02x %02x %02x",fourbytes & 0xff, (fourbytes & 0xff00) >> 8, (fourbytes & 0xff0000) >> 16, (fourbytes & 0xff000000) >> 24);

  fourbytes = 0;
  if (ControlMessageRead(0xd000, (char*)&fourbytes, 4, TIMEOUT) == FALSE) {
    Log(ERROR,"failure on 0xd000");
    fourbytes = le32_to_cpu(fourbytes);
    Log(ERROR,"0xd000 returned: %02x %02x %02x %02x",fourbytes & 0xff, (fourbytes & 0xff00) >> 8, (fourbytes & 0xff0000) >> 16, (fourbytes & 0xff000000) >> 24);
   
    return FALSE;
  }
  Log(NOTICE, "0xd000 returned: %02x %02x %02x %02x",fourbytes & 0xff, (fourbytes & 0xff00) >> 8, (fourbytes & 0xff0000) >> 16, (fourbytes & 0xff000000) >> 24);

  

  file = fopen(filename, "w");
  Log(DEBUGGING, "capture saving to %s", filename);
  if (file == NULL) {
    return FALSE;
  }
  while(1) {
    count=Read(buffer,BUFSIZE,TIMEOUT);
    x += count;
    set_bitrate(x);
    if (x - round > 65536) {
      if (flipper_capture == TRUE) {
	flipper_capture = FALSE;
	break;
      }
      round = x;
      
    }
    fwrite(buffer, 1, count, file);
  }
  EnableControls(TRUE);
  return TRUE;


}



static gboolean capture_video_start(char* filename) {
  gboolean success = FALSE;

  if (CheckCameraOpen() == FALSE)
    return FALSE;

  success = CaptureVideo(filename);
  free(filename);
  return success;
}


gboolean capture_video( GtkWidget *widget,
			GdkEvent *event,
			gpointer data) {
  GtkWidget *file_selection_dialog = NULL;
  char temporary[STRINGSIZE];
  gboolean success = FALSE;
  int i;
  FILE* file = NULL;
  char* filename_malloc;
  GError* error = NULL;
  //  GtkWidget* file_selection_box = NULL;
  
  if(CheckCameraOpen()==FALSE)
    return FALSE;
  
  //  using_dialog_mutex = TRUE;
  
  filename_malloc = get_download_filename(NULL);
  flipper_capture = FALSE;
  EnableControls(FALSE);
  if (!g_thread_create((GThreadFunc)capture_video_start, filename_malloc, FALSE, &error)) {
    Log(ERROR, "g_thread says: %s",error->message);
    EnableControls(TRUE);
    return FALSE;
  }
  MessageBoxConfirm("Hit OK or Cancel to stop recording"); //not really a choice, but eh
  
  flipper_capture = TRUE; //stops capture thread when convienent
  
 
  
  return TRUE;
  
}

