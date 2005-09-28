#include "ops-linux.h"

//static FILE* semiglobalfile;
//static gint idle_tag;
//static double global_filesize;
//file_info* videos_array[MAX_VIDEOS_ARRAY_COUNT];

file_info global_last_file;
static int GetMovieCount(void) {
  short numofvideos_le16 = 0xffff;
  short numofvideos;
  int dummy;
  
  if(ControlMessageWrite(0xb300,NULL,0,TIMEOUT)==FALSE) {
    Log("failed at 0xb3 (requesting video count).");
    Log("try unplugging camcorder and starting over");
    return(-1);
  }
  dummy = Read((char *)&numofvideos_le16,2,TIMEOUT);
  numofvideos = le16_to_cpu(numofvideos_le16);
  if(dummy < 2) {
    //    fprintf(stderr,"%d\n",numofvideos);
    Log("failed to bulk read video count");
    Log("try unplugging camcorder and starting over");
    return(-1);
  }
  return(numofvideos);
}

static constant_string GetMovieName(char * directory) {
  constant_string testname;//yes it's hackish
  FILE *in;
  int t;
  //Just a helper function to find a free movie name
  for(t=1;t<10000;t++) {
    //  testname.Format("%s\\Movie_%04d.avi",directory,t);
    snprintf(testname.text, STRINGSIZE-1, "%s/Movie_%04d.avi",directory,t);
    in=fopen(testname.text,"rb");
    if(in==NULL)
      return(testname);

    fclose(in);
  }
  return(testname);
}



static gboolean DownloadMovie(int videonumber, FILE* file) {
  
  int data, count;
  int total_data = 0;
  int alt_total;
  char buffer[BUFSIZE];
 
  // 0x9300 sets up a bulk transfer on endpoint 0x81
  data=0x0000;
  if(ControlMessageWrite(0x9300,NULL,0,TIMEOUT)==FALSE) {
    Log("failed at 0x93 (initiating bulk read of video)");
    Log("try unplugging camcorder and starting over");
    return FALSE;
  }

  //gtk_widget_set_sensitive(file_selection_box, FALSE);
  //  gtk_widget_set_sensitive(GTK_FILE_SELECTION(file_selection_box)->ok_button, FALSE);
  //  EnableControls(FALSE);
  alt_total = 0;
  total_data = 0;
  while (1) {
    //int received;
    count=Read(buffer,BUFSIZE,TIMEOUT);
    total_data += count;
    //    if (total_data - alt_total > 65536) {
      //      m_progressbar_fraction = (double)total_data / (double)filesize;
    //      alt_total = total_data;

    //    }
    
    //total_data += count;
    //if(count<1)
    //break;
    //    file.WriteHuge(buffer,count);
    //    fprintf(stderr, "fwrite %d\n",fwrite(buffer, sizeof(char), count, file));
    //gtk_progress_bar_set_orientation( m_ctl_progress, (gfloat)total_data / (gfloat)sizeofdata);

    fwrite(buffer, sizeof(char), count, file);
    
    if(count<BUFSIZE)
      break;
    //    DoMessagePump();
  }
  //  m_progressbar_fraction = 0;
  fclose(file);
  //gtk_widget_set_sensitive(file_selection_box, TRUE);
  //  EnableControls(TRUE);
  return TRUE;
  
}

static gboolean download_last_movie_start(gpointer data) {
  gboolean success = FALSE;
  FILE* semiglobalfile = data;
  //  Log("%0*zx", sizeof(size_t), (size_t)data);
  if (semiglobalfile == NULL) { 
    //    EnableControls(TRUE);
    return FALSE;
  }
  EnableControls(FALSE);
  if (DownloadMovie(GetMovieCount(),semiglobalfile) == FALSE) {
    //    strcpy(temporary, "Trouble retrieving file");
    //    if (strlen(filename) + strlen(temporary) < STRINGSIZE) {
    //      strcat(temporary, filename);
    //    }
    Log("Trouble retrieving file");
  } else {
    //    strcpy(temporary, "Success retrieving ");
    //    if (strlen(filename) + strlen(temporary) < STRINGSIZE) {
    //      strcat(temporary, filename);
    //    }
    success = TRUE;
    Log("Success retrieving file");
  }
  //  Log(temporary);
  EnableControls(TRUE);
  //  gtk_idle_remove(idle_tag);
  return success;

}


/*static gboolean download_last_movie_store_filename(GtkWidget *widget) {
  //  GtkFileSelection *file_selection_box = data;
  //  semiglobalfile = NULL;
  FILE* semiglobalfile = NULL;
  GtkWidget *file_selection_box = widget;
  constant_string c_s;
  //char filename[STRINGSIZE]; 
  char tempfilename[STRINGSIZE];
  const gchar* filename;
  GError* error = NULL;
  //Log("before");
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_box));
  Log("Filename: ");
  //  Log(filename);
  //Log("after");
  semiglobalfile = fopen(filename, "wb");
  if (semiglobalfile == NULL) {
    //reuse c_s
    strncpy(tempfilename, filename, STRINGSIZE - 3);
    strcat(tempfilename, "/.");
    
    c_s = GetMovieName(tempfilename);
    

    semiglobalfile = fopen(c_s.text, "wb");
  
    if (semiglobalfile == NULL) {

      strcpy(c_s.text,"Trouble opening ");
      if (strlen(c_s.text) + strlen(filename) < STRINGSIZE) {
	strcat(c_s.text, filename);
      }
      Log(c_s.text);
      //using_dialog_mutex = FALSE;
      //      EnableControls(TRUE);
      return FALSE;
    }
  }
  //  using_dialog_mutex = FALSE;


  //  idle_tag = gtk_idle_add(GTK_SIGNAL_FUNC(download_last_movie_start), semiglobalfile);
  if (!g_thread_create((GThreadFunc)download_last_movie_start, semiglobalfile, FALSE, &error)) {
    Log(error->message);
    return FALSE;
  }


  return TRUE;
  }*/




gboolean download_last_movie( GtkWidget *widget,
			      GdkEvent *event,
			      gpointer data) {
  GtkWidget *file_selection_dialog = NULL;
  char* filename_malloc;
  file_info* currently_selected_file = NULL;
  threesome* ts = NULL;
  
  GError* error = NULL;
  if (CheckCameraOpen() == FALSE)
    return FALSE;
  if (ChangePartition(0) == FALSE)
    return FALSE;
  if (ChangeDirectory("/DCIM/100COACH") == 0)
    return FALSE;
  //currently_selected_file = (file_info*)malloc(sizeof (file_info)); 
  currently_selected_file = &global_last_file;
  //FIXME: memory leak
  GetLastFileInfo(currently_selected_file);
  

  filename_malloc = get_download_filename(currently_selected_file->filename);
  
  ts = malloc(sizeof(ts));
  if (ts == NULL) return FALSE;

  ts->a = filename_malloc;
  ts->b = currently_selected_file->filename; //char*
  ts->c = &(currently_selected_file->filesize); //int
  EnableControls(FALSE);
  if (!g_thread_create((GThreadFunc)download_file_start_thread, ts, FALSE, &error)) {
    Log("%s",error->message);
    free(ts->a);
    free(ts);
    EnableControls(TRUE);
    return FALSE;
  }
  

  return TRUE;
  
}
