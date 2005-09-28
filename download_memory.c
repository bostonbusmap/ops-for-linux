#include "ops-linux.h"

#define FBLOCKSZ (64*1024) //64k.

static int start_global;
static int length_global;

static gboolean DownloadMemory(const char* filename, unsigned long start, unsigned long length) {
#define FBLOCKSZ (64*1024) //64k.D

  unsigned t,i,count,tcount;
  int data;
  char monitorcmd[STRINGSIZE];
  FILE* file = NULL;
  char *buffer;
  
  unsigned int fblocksz=1;	//The buffer size is softcoded so we can resize it
				//for the first transfer (cam bug workaround) and for the last
				//partial block transfer. 
  
  char Tmpfile[255];
  
  buffer=malloc(FBLOCKSZ);
  if(buffer==NULL)
    {
      Log("Couldn't allocate transfer buffer");
      return FALSE;
    }
  
  //first make sure we're on the right partition and directory!
  ChangePartition(0);
  ChangeDirectory("/DCIM");
  
  //  if (file.Open((LPCSTR)filename, CFile::modeCreate | CFile::modeWrite)==false)
  file = fopen(filename, "wb");
  if (file == NULL) {
    Log("Trouble creating %s", filename);
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
    snprintf(monitorcmd, STRINGSIZE-1, "dumpf %u %u %s", t, fblocksz, Tmpfile);
    //    monitorcmd.Format("dumpf %d %d %s",t,fblocksz,Tmpfile);
    //		Log(monitorcmd);
    if(Monitor(monitorcmd)==FALSE) {
      Log("flash dump command failed: %s", monitorcmd);
      
      fclose(file);
      free(buffer);
      return FALSE;
    }
      
    if(ControlMessageWrite(0xb101, Tmpfile, strlen(Tmpfile)+1, TIMEOUT)==FALSE) { // SetFileName
       
      Log("failed at 0xb1");
      return FALSE;
    }
      
    
    
    data=0x00;
    if(ControlMessageWrite(0x9301, (char*)&data, 0, TIMEOUT)==FALSE) { // Request File Read
      Log("failed at 0x93");
      return FALSE;
    }
    
    tcount=0;
    set_bitrate(t);
    set_progress_bar((double)t / (double)length);
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


static gboolean download_memory_start_thread(gpointer data) {
  gboolean success;
  char* var = (char*)data;
  //  EnableControls(FALSE);
  success = DownloadMemory(var, start_global, length_global);
  EnableControls(TRUE);
  free(var);
  return success;
}



static gboolean download_memory_confirmed(double_widget* d_w) {
  GError* error = NULL;
  const gchar *start_s, *length_s;
  char* filename_malloc;
//  unsigned long start, length;
  GtkWidget* file_selection_dialog = NULL;
  char saveto[STRINGSIZE];
  char* malloced_string = NULL;
  start_s = gtk_entry_get_text(GTK_ENTRY(d_w->a));
  length_s = gtk_entry_get_text(GTK_ENTRY(d_w->b));
  
  if (strlen(start_s) == 0 || strlen(length_s) == 0) {
    Log("Either the start or length fields are empty; please fill them");
    return FALSE;
  }
  start_global = atoi(start_s);
  length_global = atoi(length_s);
  
  filename_malloc = get_download_filename(NULL);  

  EnableControls(FALSE);
  if (!g_thread_create((GThreadFunc)download_memory_start_thread, filename_malloc, FALSE, &error)) {
    Log("%s",error->message);
    return FALSE;
  }

  return TRUE;
}


gboolean download_memory(GtkWidget* widget,
			 GdkEvent* event,
			 gpointer data) {
  if (CheckCameraOpen() == FALSE)
    return FALSE;
  return MessageBoxTextTwo("First box is the start location (in decimal), the second box is the length (in decimal)", download_memory_confirmed);

  //  return FALSE;
}

