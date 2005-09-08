#include "ops-linux.h"
//gchar filename_global[STRINGSIZE];


static gboolean capture_video_store_filename_sub(void) {
  flipper_capture = TRUE;
  return TRUE;
}

static gboolean capture_video_start( char* filename) {
  unsigned short int twobytes = 0x0000;
  unsigned int fourbytes = 0x00000000;
  char log_string[STRINGSIZE];
  char buffer[BUFSIZE];
  int count = 0, x = 0, round = 0;
  FILE* file = NULL;
  //  char* filename = gtk_entry_get_text(widget);
  //  char* filename = data;
  //char *fourbytes_p = (char*)fourbytes;
  if (CheckCameraOpen() == FALSE)
    return FALSE;


  //  int msg = 0x1f70;
  if (ControlMessageWrite(0x1f30, (int*)twobytes, 0, TIMEOUT) == FALSE) {
    Log("failure on 0x1f30");
    return FALSE;
  }
  fourbytes = 0;
  /*count = usb_control_msg(m_p_handle,
			  //USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_INTERFACE,
			  0xc0,
			  
			  0x01,
			  0xd000,
			  0x0000,
			  (char *)&fourbytes,
			  4,
			  TIMEOUT );*/
  //fprintf(stderr,"count = %d\n",count);

  if (ControlMessageRead(0xd000, (int*)&fourbytes, 4, TIMEOUT) == FALSE) {
    Log("failure on 0xd000");
    snprintf(log_string, STRINGSIZE - 1, "0xd000 returned: %02x %02x %02x %02x",fourbytes & 0xff, (fourbytes & 0xff00) >> 8, (fourbytes & 0xff0000) >> 16, (fourbytes & 0xff000000) >> 24);
    Log(log_string);
    return FALSE;
  }
  snprintf(log_string, STRINGSIZE - 1, "0xd000 returned: %02x %02x %02x %02x",fourbytes & 0xff, (fourbytes & 0xff00) >> 8, (fourbytes & 0xff0000) >> 16, (fourbytes & 0xff000000) >> 24);
  Log(log_string);

  if (ControlMessageWrite(0xf002, (int*)twobytes, 0, TIMEOUT) == FALSE) {
    Log("failure on 0xf002");
    return FALSE;
  }
  if (ControlMessageWrite(0x2000, (int*)twobytes, 0, TIMEOUT) == FALSE) {
    Log("failure on 0x2000");
    return FALSE;
  }
  if (ControlMessageWrite(0x2f0f, (int*)twobytes, 0, TIMEOUT) == FALSE) {
    Log("failure on 0x2f0f");
    return FALSE;
  }
  if (ControlMessageWrite(0x2650, (int*)twobytes, 0, TIMEOUT) == FALSE) {
    Log("failure on 0x2650");
    return FALSE;
  }
  if (ControlMessageWrite(0xe107, (int*)twobytes, 0, TIMEOUT) == FALSE) {
    Log("failure on 0xe107");
    return FALSE;
  }
  twobytes = 0xffff;
  if (ControlMessageWrite(0x2502, (int*)&twobytes, 2, TIMEOUT) == FALSE) {
    Log("failure on 0x2502");
    return FALSE;
  }
  
  if (ControlMessageWrite(0x1f70, (int*)twobytes, 0, TIMEOUT) == FALSE) {
    Log("failure on 0x1f70");
    return FALSE;
  }
  fourbytes = 0;

  if (ControlMessageRead(0xd000, (int*)&fourbytes, 0, TIMEOUT) == FALSE) {
    Log("failure on 0xd000");
    snprintf(log_string, STRINGSIZE - 1, "0xd000 returned: %02x %02x %02x %02x",fourbytes & 0xff, (fourbytes & 0xff00) >> 8, (fourbytes & 0xff0000) >> 16, (fourbytes & 0xff000000) >> 24);
    Log(log_string);
    return FALSE;
  }
  snprintf(log_string, STRINGSIZE - 1, "0xd000 returned: %02x %02x %02x %02x",fourbytes & 0xff, (fourbytes & 0xff00) >> 8, (fourbytes & 0xff0000) >> 16, (fourbytes & 0xff000000) >> 24);

  Log(log_string);
  fourbytes = 0;
  if (ControlMessageRead(0xd000, (int*)&fourbytes, 4, TIMEOUT) == FALSE) {
    Log("failure on 0xd000");
    snprintf(log_string, STRINGSIZE - 1, "0xd000 returned: %02x %02x %02x %02x",fourbytes & 0xff, (fourbytes & 0xff00) >> 8, (fourbytes & 0xff0000) >> 16, (fourbytes & 0xff000000) >> 24);
    Log(log_string);
    return FALSE;
  }
  snprintf(log_string, STRINGSIZE - 1, "0xd000 returned: %02x %02x %02x %02x",fourbytes & 0xff, (fourbytes & 0xff00) >> 8, (fourbytes & 0xff0000) >> 16, (fourbytes & 0xff000000) >> 24);

  Log(log_string);

  //count=Read(buffer,BUFSIZE,TIMEOUT);
  //snprintf(log_string, STRINGSIZE - 1, "%d bytes read successfully", count);
  //Log(log_string);
  //file = fopen("testmonkey","w");
  file = fopen(filename, "w");
  Log(filename);
  if (file == NULL) {
    return FALSE;
  }

  //EnableControls(FALSE);
  while(1) {
    count=Read(buffer,BUFSIZE,TIMEOUT);
    //Log("reading...");
    //if(count<BUFSIZE)
    //break;
    x += count;
    set_bitrate(x);
    if (x - round > 65536) {
      //gtk_progress_bar_set_fraction(m_ctl_progress, (gdouble)x / (gdouble)filesize);
      //      m_progressbar_fraction = (double)x / (double)filesize;
      if (flipper_capture == TRUE) {
	flipper_capture = FALSE;
	break;
      }
      round = x;
      //gtk_widget_show(main_window); //update?
      
    }
    
    //    printf("x: %d\n", x);
    //    file.WriteHuge(buffer,count);
    fwrite(buffer, 1, count, file);
    //fprintf(stderr,"%d bytes, %d bytes total\n",count,x);
    //  break;
    //DoMessagePump();
  }
  EnableControls(TRUE);
  return TRUE;


  /*  if (toggle_camera_lcd_screen_is_on == TRUE)
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
  */
}

static gboolean capture_video_store_filename(GtkFileSelection* file_selection_box) {
  GError* error = NULL;
  gchar* filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_box));
  flipper_capture = FALSE;
  EnableControls(FALSE);
  if (!g_thread_create(capture_video_start, filename, FALSE, &error)) {
    Log(error->message);
    EnableControls(TRUE);
    return FALSE;
  }
  MessageBoxChoice("Hit OK or Cancel to stop recording", capture_video_store_filename_sub);
  
  
  return TRUE;

}



gboolean capture_video( GtkWidget *widget,
			GdkEvent *event,
			gpointer data) {
  GtkWidget *file_selection_box = NULL;
  char temporary[STRINGSIZE];
  gboolean success = FALSE;
  int i;
  FILE* file = NULL;
  
 
  //  GtkWidget* file_selection_box = NULL;
  
  if(CheckCameraOpen()==FALSE)
    return FALSE;

  //  using_dialog_mutex = TRUE;

  file_selection_box = gtk_file_selection_new("Please select a file to save capture information");
  
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->ok_button),
			     "clicked",
			     GTK_SIGNAL_FUNC (capture_video_store_filename),
			     file_selection_box);
  
  /*  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->ok_button),
		      "clicked",
		      GTK_SIGNAL_FUNC (start_download_file),
		      NULL);*/

  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->ok_button),
			     "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     (gpointer) (file_selection_box));
  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->cancel_button),
		      "clicked",
		      GTK_SIGNAL_FUNC (enable_buttons),
		      (gpointer) file_selection_box);

  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->cancel_button),
			     "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     (gpointer) file_selection_box);
  

  gtk_widget_show(file_selection_box);
  //EnableControls(FALSE);
  //  for (i=0;i<100000;++i);
  //if the program's going to stall it might as well be here

  
  return TRUE;
  
}

