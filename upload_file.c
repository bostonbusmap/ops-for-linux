#include "ops-linux.h"
//gint idle_tag;

file_info* global_p;
gchar* global_filename;
char storedir_global[STRINGSIZE];
int GetLength(FILE* fp) {
  int l;
  fseek(fp, 0, SEEK_END);
  l = ftell(fp);
  
  fseek(fp, 0, SEEK_SET);
  return l;
}

gboolean UploadFile(char* saveto, char* filename, file_info* p) {
	// This function expects that you've already changed the parition and 
	// directory to the correct path in the camcorder. dirpath is the Windows
	// path to the file to store in.

  int count,x, tcount, alt_total;
  int data;
#ifdef _WIN32
  char delimiter = '\\';
#else
  char delimiter = '/';
#endif
  FILE* file = NULL;
  //char* sfilename = filename;
  unsigned char sfilename[256];
  unsigned char tempfilename[STRINGSIZE];
  unsigned char* pTempfilename = tempfilename;
  unsigned char buffer[BUFSIZE];
  char* directory;
  unsigned int udata;
  int filesize = 0;// p->filesize;
  strcpy(tempfilename, filename);
  //printf("DownloadFile(%s, %s, %s)\n",saveto,filename);
  Log("in UploadFile");
  if (p->filetype != FIFILE) { //FIDIR or FIPART
    //    char* endTempfilename = tempfilename + strlen(tempfilename);
    //    strcpy(tempfilename, p->filetype);
    strcpy(tempfilename, saveto);
    pTempfilename = tempfilename + strlen(tempfilename);
    --pTempfilename;
    while ((pTempfilename != tempfilename) && (*pTempfilename != delimiter))
      --pTempfilename;
    ++pTempfilename; 

    fprintf(stderr,"not file pTempfilename: %s\n", pTempfilename);
    if (strlen(pTempfilename) > 12) {
      MessageBox("Sorry, long filenames are not supported.");
      return FALSE;
    }
    directory = p->fullpath;
  } else {
    strcpy(tempfilename, p->filename);
    pTempfilename = tempfilename + strlen(tempfilename);
    --pTempfilename;
    while ((pTempfilename != tempfilename) && (*pTempfilename != '/'))
      --pTempfilename;
    ++pTempfilename;
    Log("p->filename == FIFILE");
    Log(tempfilename);
    directory = p->dirpath;
  }

  if(ChangePartition(p->partition)==FALSE) {
    Log("ChangePartition(p->partition) failed for upload.");
    return FALSE;
  }

  Log("directory ==");
  Log(directory);
  if(ChangeDirectory(directory)== FALSE) {
    Log("ChangeDirectory(directory) failed for upload.");
    return FALSE;
  }

  //  EnableControls(FALSE);
  


  //  strcpy(sfilename, filename);
  file = fopen(saveto, "r");
  if (file == NULL) {

    Log("Trouble opening: ");
    Log(filename);
    return FALSE;
  }
  
  
  memset(sfilename,0,255);
  strncpy((char *)sfilename,pTempfilename, 12); //being extra careful?
  
  Log("sfilename ==");
  Log(sfilename);
  if(ControlMessageWrite(0xb100,(int *)sfilename,strlen((char *)sfilename)+1,TIMEOUT)==FALSE) { //SetFileName
    Log("failed at 0xb1");
    return FALSE;
  }
  //Endianness appears to be screwed with this command!!
  udata=((GetLength(file) & 0xffff)<<16)|((GetLength(file)>>16));
  filesize = GetLength(file); //please please don't call this function 3 times, compiler

  
  //data=0x00;
  if(ControlMessageWrite(0x9500,(int*)&udata,4,TIMEOUT)==FALSE) { // Request Write
    Log("failed at 0x95");
    return FALSE;
  }

  tcount = 0;
  alt_total = 0;
  //NOTE: we keep grabbing bytes until the camcorder is done feeding us.
  x=0;
  //  EnableControls(FALSE);
  while(1) {
    //    Log("writing...");
    count = fread(buffer, sizeof(char), BUFSIZE, file);
    Log("read...");
    if (count < 1) break;
    tcount += count;
    //    fprintf(stderr,"count: %d\n",count);
    if (tcount  - alt_total > 65536) {
      m_progressbar_fraction = (double)tcount / (double)filesize;
      //      fprintf(stderr,"m_progressbar: %f\n",m_progressbar_fraction);
      alt_total = tcount;
    }
    //printf("tcount: %d count: %d\n", tcount, count);
    x=Write(buffer,count,TIMEOUT);
    //printf("x: %d\n",x);
    if(x<1) {
      Log("Error writing file to camera");
      fclose(file);
      //EnableControls(TRUE);
      m_progressbar_fraction = 0;
      return FALSE;
    }

    //Log("reading...");
    //if(count<BUFSIZE)
    //break;
    //x += count;
    //    printf("x: %d\n", x);
    //    file.WriteHuge(buffer,count);
    //    if(count<BUFSIZE)
    //break;
    //DoMessagePump();
  }

  //CString msg; msg.Format("file was %d bytes in size",x); Log(msg);
  m_progressbar_fraction = 0;

  fclose(file);
  Log("Success sending file");
  //EnableControls(TRUE);
  return TRUE;


}

gboolean upload_file_start( gpointer data) {
  file_info* p = global_p;
  gboolean success = FALSE;
  //  string_combo* s_c = data;
  char* filename = data;
  //  char* dirpath = s_c.b;
  char tempstring[STRINGSIZE];
  /*  if (semiglobalfile == NULL) { 
    EnableControls(TRUE);
    return FALSE;
    }*/
  
  Log("starting UploadFile(storedir_global, filename, p");
  fprintf(stderr, "%s, %s, %s\n",storedir_global, filename, p);
  EnableControls(FALSE);
  if(UploadFile(storedir_global,filename, p)==FALSE) {
    Log("UploadFile(p->filename, tempstring) failed.");
    EnableControls(TRUE);
    return FALSE;
  } else {
    Log("Success sending data file.");
  }
  EnableControls(TRUE);


  //  Log(temporary);
  //EnableControls(TRUE);
  //  gtk_idle_remove(idle_tag);
  return success;

}



gboolean upload_file_store_filename (GtkWidget *widget) {
  GtkWidget* file_selection_box = widget;
  GtkTreeSelection* selection;
  //  file_info* currently_selected_item;
  GtkTreeModel* model;// = m_directory_model;
  GtkTreeIter iter;
  gpointer data = NULL;
  
  //  FILE* semiglobalfile = NULL;
  
  if(CheckCameraOpen()== FALSE)
    return FALSE;
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_directory_tree));
  if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    //    gpointer data = NULL;
    file_info* p = NULL;
    gchar* winfilename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_box));
    gchar* filename;
    char tempstring[STRINGSIZE];
    //char [STRINGSIZE];
    //    char* storedir = dirpath;
    //string_combo s_c;
    
    strncpy(storedir_global, winfilename, STRINGSIZE - 1);

    
    gtk_tree_model_get (model, &iter, COL_FILENAME, &filename,
			COL_POINTER, &data, -1);
    p = data;
    //	m_file_info = "";

       
    //	FILE_INFO* p = (FILE_INFO*)m_directory_tree.GetItemData(hItem);

    if (p) {
      if(p->filetype==FIROOT) {
	MessageBox("Sorry, can't upload to the meta root.");
	return FALSE;
      }
      global_p = p; //pass this to upload_file_confirmed
      global_filename = filename;
      if (p->partition != 0) {
	return MessageBoxChoice("Do you really want to upload a file to a non-movie partition?\nThis could easily kill your camera.", upload_file_confirmed);
	
	
      }
      return MessageBoxChoice("Are you sure?", upload_file_confirmed); //widget value doesn't matter here
      
      
      //Ok, looks like this is going to work! Let the user pick the storage dir
      //		CString storedir=BrowseForFolder();
      //if(storedir=="")
      //return;
      //storedir.TrimRight("\\");
    }
  }
  //  EnableControls(TRUE);
  return TRUE;
}

gboolean upload_file_confirmed (GtkWidget *widget) { 
  file_info* p = global_p;
  gchar* filename = global_filename;
  GError* error = NULL;
      ////

  //  EnableControls(FALSE);
  
  Log("Attempting to send: ");
  Log(p->filename);
  //dirpath_global = p->dirpath;
  //  idle_tag = gtk_idle_add(GTK_SIGNAL_FUNC(upload_file_start), p->filename);
  if (!g_thread_create(upload_file_start, p->filename, FALSE, &error)) {
    Log(error->message);
    return FALSE;
  }


  
      //I know it's messy to send 2 global variables and a local one
  
  //Log();
  //return FALSE; //no leaf in the tree is selected OR data can't be retrieved OR we're done..
  
  
  //EnableControls(TRUE);
  return TRUE;
  

  
  

}


gboolean upload_file( GtkWidget *widget,
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

  file_selection_box = gtk_file_selection_new("Please select a file to upload");
  
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->ok_button),
			     "clicked",
			     GTK_SIGNAL_FUNC (upload_file_store_filename),
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
  //  EnableControls(FALSE);
  //  for (i=0;i<100000;++i);
  //if the program's going to stall it might as well be here

  
  return TRUE;
  
}

