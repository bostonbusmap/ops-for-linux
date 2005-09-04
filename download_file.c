#include "ops-linux.h"

static int global_filesize;

static char storedir_global[STRINGSIZE];

gboolean DownloadFile(char* saveto, char* filename) {
	// This function expects that you've already changed the parition and 
	// directory to the correct path in the camcorder. dirpath is the Windows
	// path to the file to store in.
  int filesize = global_filesize;
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

  //EnableControls(FALSE);

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

  //NOTE: we keep grabbing bytes until the camcorder is done feeding us.
  x=0;
  round = 0;
  if (m_ctl_progress) gtk_progress_bar_set_fraction(m_ctl_progress, 0);
  while(1) {
    count=Read(buffer,BUFSIZE,TIMEOUT);
    //Log("reading...");
    //if(count<BUFSIZE)
    //break;
    x += count;
    if (x - round > 65536) {
      //if (m_ctl_progress) gtk_progress_bar_set_fraction(m_ctl_progress, (gdouble)x / (gdouble)filesize);
      m_progressbar_fraction = (double)x / (double)filesize;
      round = x;
      //gtk_widget_show(main_window); //update?
    }
    
    //    printf("x: %d\n", x);
    //    file.WriteHuge(buffer,count);
    fwrite(buffer, 1, count, file);
    if(count<BUFSIZE)
      break;
    //DoMessagePump();
  }
  
  //CString msg; msg.Format("file was %d bytes in size",x); Log(msg);
  

  fclose(file);
  Log("Success retrieving file");
  //EnableControls(TRUE);
  return TRUE;


}

static gboolean download_file_start( gpointer data) {
  gboolean success = FALSE;
  //  string_combo* s_c = data;
  char* filename = data;
  //  char* dirpath = s_c.b;
  char tempstring[STRINGSIZE];
  /*  if (semiglobalfile == NULL) { 
    EnableControls(TRUE);
    return FALSE;
    }*/
  //gdk_threads_enter();
  EnableControls(FALSE);
  if(DownloadFile(storedir_global,filename)==FALSE) {
    Log("DownloadFile(p->filename, tempstring) failed.");
    //    EnableControls(TRUE);
    //gdk_threads_leave();
    EnableControls(TRUE);
    return FALSE;
  } else {
    Log("Success retrieving data file.");
  }
  //  EnableControls(TRUE);




  EnableControls(TRUE);
  m_progressbar_fraction = 0;
  //gdk_threads_leave();
  //  Log(temporary);
  //EnableControls(TRUE);

  return success;

}



static gboolean download_file_store_filename (GtkWidget *widget) {
  GError *error = NULL;
  GtkWidget* file_selection_box = widget;
  GtkTreeSelection* selection;
  file_info* currently_selected_item;
  GtkTreeModel* model;// = m_directory_model;
  GtkTreeIter iter;
  int round;
  //  FILE* semiglobalfile = NULL;
  
  if(CheckCameraOpen()== FALSE)
    return FALSE;
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_directory_tree));
  if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    gpointer data;
    file_info* p;
    const gchar *filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_box));
    //char [STRINGSIZE];
    //    char* storedir = dirpath;
    //string_combo s_c;
    
    strncpy(storedir_global, filename, STRINGSIZE - 1);

    
    gtk_tree_model_get (model, &iter, COL_FILENAME, &filename,
			COL_POINTER, &data, -1);
    p = data;
    global_filesize = p->filesize;
    //	m_file_info = "";

       
    //	FILE_INFO* p = (FILE_INFO*)m_directory_tree.GetItemData(hItem);
    gtk_progress_bar_set_fraction( m_ctl_progress, 0);
    round = 0;
    if (p) {
      if(p->filetype!=FIFILE) {
	MessageBox("Sorry, downloading directory/partitions is not supported");
	return FALSE;
      }
      
      //Ok, looks like this is going to work! Let the user pick the storage dir
      //		CString storedir=BrowseForFolder();
      //if(storedir=="")
      //return;
      //storedir.TrimRight("\\");
      

      if(ChangePartition(p->partition)==FALSE) {
	Log("ChangePartition(p->partition) failed.");
	return FALSE;
      }

      
      if(ChangeDirectory(p->dirpath)== FALSE) {
	Log("ChangeDirectory(p->dirpath) failed.");
	return FALSE;
      }


      ////

      //  EnableControls(FALSE);
      
      Log("Attempting to retrieve: ");
      Log(p->filename);
      //dirpath_global = p->dirpath;
      //gtk_idle_add(GTK_SIGNAL_FUNC(download_file_start), p->filename);
      if (!g_thread_create(download_file_start, p->filename, FALSE, &error)) {
	Log(error->message);
	return FALSE;
      }

      
      //I know it's messy to send 2 global variables and a local one
    }
    //Log();
    //return FALSE; //no leaf in the tree is selected OR data can't be retrieved OR we're done..

  }
  //EnableControls(TRUE);
  return TRUE;





}


gboolean download_file( GtkWidget *widget,
			GdkEvent *event,
			gpointer data) {
  GtkWidget *file_selection_box = NULL;
 
  //  GtkWidget* file_selection_box = NULL;
  
  if(CheckCameraOpen()==FALSE)
    return FALSE;

  //  using_dialog_mutex = TRUE;

  file_selection_box = gtk_file_selection_new("Please select a file to download");


  // fill in name of file for easy clicking
  GtkTreeModel* model;
  GtkTreeIter iter;
  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_directory_tree));
  if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    gchar* filename = NULL;
    gtk_tree_model_get (model, &iter, COL_FILENAME, &filename, -1);
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection_box),
		                    filename);
  }
  
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->ok_button),
			     "clicked",
			     GTK_SIGNAL_FUNC (download_file_store_filename),
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

