#include "ops-linux.h"

//static FILE* semiglobalfile;
//static gint idle_tag;
//static double global_filesize;

static int GetMovieCount(void) {
  short numofvideos_le16 = 0xffff;
  short numofvideos;
  int dummy;
  
  if(ControlMessageWrite(0xb300,&dummy,0,TIMEOUT)==FALSE) {
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




/*
static gboolean download_last_movie_start(gpointer data) {
  gboolean success = FALSE;
  FILE* semiglobalfile = data;
  printf("%0*zx\n", sizeof(size_t), (size_t)data);
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


static gboolean download_last_movie_store_filename(GtkWidget *widget) {
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
  semiglobalfile = fopen(filename, "w");
  if (semiglobalfile == NULL) {
    //reuse c_s
    strncpy(tempfilename, filename, STRINGSIZE - 3);
    strcat(tempfilename, "/.");
    
    c_s = GetMovieName(tempfilename);
    

    semiglobalfile = fopen(c_s.text, "w");
  
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
  if (!g_thread_create(download_last_movie_start, semiglobalfile, FALSE, &error)) {
    Log(error->message);
    return FALSE;
  }


  return TRUE;
}

*/


gboolean download_last_movie( GtkWidget *widget,
			      GdkEvent *event,
			      gpointer data) {
  /*
  GtkWidget *file_selection_box = NULL;
  
 
  //  GtkWidget* file_selection_box = NULL;
  
  if(CheckCameraOpen()==FALSE)
    return FALSE;

  //  using_dialog_mutex = TRUE;
  file_selection_dialog = gtk_file_chooser_dialog_new("Please select a place to download to",
						      widget, //meaingless?
						      GTK_FILE_CHOOSER_ACTION_SAVE,
						      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
						      NULL);
  
  if (gtk_dialog_run(GTK_DIALOG(file_selection_dialog)) == GTK_RESPONSE_ACCEPT) {
    save_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_selection_dialog));
  } else { //user cancelled
    return FALSE;
  }


  EnableControls(FALSE);
  if(DownloadFile(save_filename,currently_selected_file->filename)==FALSE) {
    Log("DownloadFile(p->filename, tempstring) failed.");
    EnableControls(TRUE);
    return FALSE;
  } else {
    Log("Success retrieving data file.");
  }


  EnableControls(TRUE);
  set_progress_bar(0.0);


  
  if(currently_selected_file->filetype!=FIFILE) {
    MessageBox("Sorry, downloading directory/partitions is not supported");
    return FALSE;
  }
  
  if(ChangePartition(currently_selected_file->partition)==FALSE) {
    Log("ChangePartition(p->partition) failed.");
    return FALSE;
  }

  EnableControls(FALSE);
  if(DownloadFile(currently_selected_file->filename,save_filename)==FALSE) {
    Log("DownloadFile(p->filename, tempstring) failed.");
    EnableControls(TRUE);
    return FALSE;
  } else {
    Log("Success retrieving data file.");
  }
  
  
  
  
  EnableControls(TRUE);
  set_progress_bar(0.0);
  return TRUE;

  
  return TRUE;*/
  
}


