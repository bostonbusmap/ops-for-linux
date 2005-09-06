#include "ops-linux.h"

//static int global_filesize;

//static char storedir_global[STRINGSIZE];

gboolean DownloadFile(char* saveto, char* filename, int filesize) {
	// This function expects that you've already changed the parition and 
	// directory to the correct path in the camcorder. dirpath is the Windows
	// path to the file to store in.
  //  int filesize = global_filesize;
  int count,x;
  int data;
  int round;
  FILE* file = NULL;
  //char* sfilename = filename;
  char sfilename[256];
  char buffer[BUFSIZE];
  printf("DownloadFile(%s, %s)\n", saveto, filename);
  strcpy(sfilename, filename);
  file = fopen(saveto, "w");
  if (file == NULL) {
    Log("Trouble opening: ");
    Log(filename);
    return FALSE;
  }


  memset(sfilename, '\0', sizeof sfilename);
  strcpy(sfilename,filename);

  if(ControlMessageWrite(0xb101, sfilename, strlen(sfilename)+1, TIMEOUT)==FALSE) { //SetFileName
    Log("failed at 0xb1");
    EnableControls(TRUE);
    return FALSE;
  }
  
  
  
  data=0x00;
  if(ControlMessageWrite(0x9301,&data,0,TIMEOUT)==FALSE) { // Request File Read
    Log("failed at 0x93");
    return FALSE;
  }

  x=0;
  round = 0;
  while(1) {
    count=Read(buffer,BUFSIZE,TIMEOUT);
    x += count;
    if (x - round > 65536) {
      set_progress_bar((double)x / (double)filesize);
      round = x;
    }
    fwrite(buffer, 1, count, file);
    if(count<BUFSIZE)
      break;
  }
  
  

  fclose(file);
  Log("Success retrieving file");
  return TRUE;


}


gboolean download_file( GtkWidget *widget,
			GdkEvent *event,
			gpointer data) {
  GtkWidget *file_selection_dialog = NULL;
  char* save_filename;
  //  GtkWidget* file_selection_box = NULL;
  
  // fill in name of file for easy clicking
  GtkTreeModel* model;
  GtkTreeIter iter;
  GtkTreeSelection* selection;
  //  gpointer data;
  file_info* currently_selected_file;
  if(CheckCameraOpen()==FALSE)
    return FALSE;

  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_directory_tree));
  if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    //    gchar* filename = NULL;
    gtk_tree_model_get(model, &iter, COL_POINTER, &currently_selected_file, -1);
    if (currently_selected_file == NULL) {//not too likely
      return FALSE;
    }
  } else {
    return FALSE; //nothing selected for download
  }



  file_selection_dialog =
    gtk_file_chooser_dialog_new("Please select a place to download to",
				widget, //meaingless except if program killed
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);
  gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(file_selection_dialog),currently_selected_file->filename );
  if (gtk_dialog_run(GTK_DIALOG(file_selection_dialog)) == GTK_RESPONSE_ACCEPT) {
    save_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_selection_dialog));
  } else { //user cancelled
    return FALSE;
  }
  
  if(currently_selected_file->filetype!=FIFILE) {
    MessageBox("Sorry, downloading directory/partitions is not supported");
    return FALSE;
  }
  
  if(ChangePartition(currently_selected_file->partition)==FALSE) {
    Log("ChangePartition(p->partition) failed.");
    return FALSE;
  }

  if(ChangeDirectory(currently_selected_file->dirpath)== FALSE) {
    Log("ChangeDirectory(p->dirpath) failed.");
    return FALSE;
  }


  EnableControls(FALSE);
  if(DownloadFile(currently_selected_file->filename,save_filename, currently_selected_file->filesize)==FALSE) {
    Log("DownloadFile(p->filename, tempstring) failed.");
    EnableControls(TRUE);
    return FALSE;
  } else {
    Log("Success retrieving data file.");
  }
  
  
  
  
  EnableControls(TRUE);
  set_progress_bar(0.0);
  return TRUE;

    
}

