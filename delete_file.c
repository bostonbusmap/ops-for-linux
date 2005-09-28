#include "ops-linux.h"

//static file_info* global_fi;

static gboolean DelFile(const char* name) {
  char buffer[255];
  memset(buffer, '\0', sizeof buffer);
  snprintf(buffer, sizeof buffer, "del %s", name);
  if(ControlMessageWrite(0xef00,buffer,strlen(buffer)+1,TIMEOUT)==TRUE) {
    return TRUE;
  }
  Log("delete failed");
  return FALSE;
}

gboolean delete_file(GtkWidget* widget,
		     GdkEvent* event,
		     gpointer data) {
  GtkTreeSelection* selection;
  GtkTreeModel* model;// = m_directory_model;
  GtkTreeIter iter;
  char tempstring[STRINGSIZE];
  
  if(CheckCameraOpen()==FALSE)
    return FALSE;
  
  //HTREEITEM hItem = m_directory_tree.GetSelectedItem();
  
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_directory_tree));
  if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    //    gpointer data = NULL;
    file_info* p = NULL;
    //    gchar* filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_box));
    gchar* filename;
    //char [STRINGSIZE];
    //    char* storedir = dirpath;
    //string_combo s_c;
    gtk_tree_model_get (model, &iter, COL_FILENAME, &filename,
			COL_POINTER, &data, -1);
    p = data;
   
    if (p) {
      if(p->filetype!=FIFILE) {
	MessageBox("Sorry, delete is currently limited to files");
	return FALSE;
      }
      
      if(p->partition!=0) {
	if (MessageBoxConfirm("Do you really want delete a file from a non-movie partition?\r\nThis could easily kill your camera.") == FALSE)
	  return FALSE;
      } else {
	strcpy(tempstring, "Do you really want to delete ");
	if (strlen(tempstring) + strlen(p->filename) + 1 < STRINGSIZE) {
	  strcat(tempstring,p->filename);
	  strcat(tempstring,"?");
	}
	if (MessageBoxConfirm(tempstring) == FALSE) {
	  return FALSE;
	}
	
	if(ChangePartition(p->partition)==FALSE) {
	  Log("ChangePartition failed in delete_file_confirmed");
	  return FALSE;
	}
	  
	if(ChangeDirectory(p->dirpath)==FALSE) {
	  Log("ChangeDirectory failed in delete_file_confirmed");
	  return FALSE;
	}
	
	EnableControls(FALSE);
	  
	if(DelFile(p->filename)==FALSE) {
	  strcpy(tempstring, "Failed to delete ");
	  if (strlen(tempstring) + strlen(p->filename) < STRINGSIZE) {
	    strcat(tempstring, p->filename);
	  }
	  //Log("Failed to delete "+p->filename);
	  Log(tempstring);
	  EnableControls(TRUE);
	  return FALSE;
	}
	strcpy(tempstring, "Deleted ");
	if (strlen(tempstring) + strlen(p->filename) + strlen("successfully") < STRINGSIZE) {
	  strcat(tempstring, p->filename);
	  strcat(tempstring, "successfully");
	}
	Log(tempstring);
	EnableControls(TRUE);
      }
    }
  } 
  return TRUE;
}

