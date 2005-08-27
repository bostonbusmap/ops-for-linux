#include "ops-linux.h"

#define FBLOCKSZ (64*1024) //64k.

static int start_global;
static int length_global;

static gboolean DownloadMemory(const char* filename, unsigned long start, unsigned long length) {
#define FBLOCKSZ (64*1024) //64k.D

  unsigned long int t,i,count,tcount;
  int data;
  char monitorcmd[STRINGSIZE];
  FILE* file = NULL;
  unsigned char *buffer;
  
  unsigned int fblocksz=1;	//The buffer size is softcoded so we can resize it
				//for the first transfer (cam bug workaround) and for the last
				//partial block transfer. 
  
  char Tmpfile[255];
  
  buffer=(unsigned char *)malloc(FBLOCKSZ);
  if(buffer==NULL)
    {
      Log("Couldn't allocate transfer buffer");
      return FALSE;
    }
  
  //first make sure we're on the right partition and directory!
  ChangePartition(0);
  ChangeDirectory("/DCIM");
  
  //  if (file.Open((LPCSTR)filename, CFile::modeCreate | CFile::modeWrite)==false)
  file = fopen(filename, "w");
  if (file == NULL) {
    Log("Trouble creating: ");
    Log(filename);
    free(buffer);
    return FALSE;
  }
  
  
  i=0;
  for(t=start;t<start+length;t=t+fblocksz) {
    fblocksz=FBLOCKSZ;
    
    if(t+FBLOCKSZ>start+length) //just download the last portion of a block
      fblocksz=(start+length)-t;
      
    if(t==0)
      fblocksz=4; //we always short a transfer that starts at 0 to
    //workaround a cam bug.
    
    i++;
    memset(Tmpfile,0,255);
    sprintf(Tmpfile,"%07x.BIN",1);
    snprintf(monitorcmd, STRINGSIZE - 1,"dumpf %d %d %s",t,fblocksz,Tmpfile);
    //    monitorcmd.Format("dumpf %d %d %s",t,fblocksz,Tmpfile);
    //		Log(monitorcmd);
    if(Monitor(monitorcmd)==FALSE) {
      Log("flash dump command failed: ");
      Log(monitorcmd);
      fclose(file);
      free(buffer);
      return FALSE;
    }
      
    if(ControlMessageWrite(0xb101,(int *)Tmpfile,strlen((char *)Tmpfile)+1,TIMEOUT)==FALSE) { // SetFileName
       
      Log("failed at 0xb1");
      return FALSE;
    }
      
    
    
    data=0x00;
    if(ControlMessageWrite(0x9301,&data,0,TIMEOUT)==FALSE) { // Request File Read
      Log("failed at 0x93");
      return FALSE;
    }
    
    tcount=0;
    while (1) {
      count=Read(buffer,BUFSIZE,TIMEOUT);
      if(count<1)
	break;
      tcount+=count;
      //file.WriteHuge(buffer,count);
      fwrite(buffer,sizeof(char), count, file);
      if(count<BUFSIZE)
	break;
      //DoMessagePump();
    }
      
    if(tcount<fblocksz) {
      Log("short buffer error. data will likely be corrupt");
      free(buffer);
      fclose(file);
      return FALSE;
    }
      
      
    //      DoMessagePump();
      
  }
  fclose(file);
  Log("memory download succeeded");
  free(buffer);
  return TRUE;


}


static gboolean download_memory_start(gpointer data) {
  return DownloadMemory(data, start_global, length_global);
}

static gboolean download_memory_store_filename(GtkFileSelection* file_selection_box) {
  GError *error;
  gchar* winfilename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_box));
  if (!g_thread_create(download_memory_start, winfilename, FALSE, &error)) {
    Log(error->message);
    return FALSE;
  }

  return TRUE;
}

static gboolean download_memory_confirmed(double_widget* d_w) {
  gchar *start_s, *length_s;
  unsigned long start, length;
  GtkFileSelection* file_selection_box;
  start_s = gtk_entry_get_text(d_w->a);
  length_s = gtk_entry_get_text(d_w->b);
  
  if (strlen(start_s) == 0 || strlen(length_s) == 0) {
    Log("Either the start or length fields are empty; please fill them");
    return FALSE;
  }
  start_global = atoi(start_s);
  length_global = atoi(length_s);
  

  file_selection_box = gtk_file_selection_new("Please choose the filename to download to");
  
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->ok_button),
			     "clicked",
			     GTK_SIGNAL_FUNC (download_memory_store_filename),
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



}


gboolean download_memory(GtkWidget* widget,
			 GdkEvent* event,
			 gpointer data) {
  if (CheckCameraOpen() == FALSE)
    return FALSE;
  return MessageBoxTextTwo("First box is the start location (in decimal), the second box is the length (in decimal)", download_memory_confirmed);

  //  return FALSE;
}

gboolean Monitor(const char* command) {
  
  FILE* file;
  unsigned char buffer[255];


  memset(buffer,0,255);
  strcpy((char *)buffer,command);
  if(ControlMessageWrite(0xef00,(int *)buffer,strlen((char *)buffer)+1,LONG_TIMEOUT)==TRUE) {
    Log("monitor command succeeded.");
    return TRUE;
  }
  Log("monitor command failed");
  return FALSE;

}
