#include "ops-linux.h"

gboolean WaitMediaStatus() {
  //	Another experimental function
  unsigned int cmd;
  unsigned char data[10];
  
  return TRUE;	//can't seem to get this to ensure media is available
					//maybe it does something else



  cmd=0xb600;
  if(ControlMessageRead(cmd,(int *)data,10,TIMEOUT)) {
    return TRUE;
  }
  
  return FALSE;

}



gboolean GetFileInfo(file_info* thisfileinfo, gboolean isfirstfile) {
  
  unsigned char data[29],tempname[29];
  int dummy,command,s,t;
  memset(data,0,29);


  if(isfirstfile==TRUE)
    command=0xbc00; //first file

  if(isfirstfile==FALSE)
    command=0xbd00; //next file
  
  //sometimes the first-file read fails after a partition change
  for(t=0;t<2;t++) {
    if(ControlMessageWrite(command,&dummy,0,TIMEOUT)==FALSE) {
      Log("failed at 0xbc (requesting first file info).");
      return FALSE;
    }
    if(((s=Read(data,28,TIMEOUT)))<28) {
      if(isfirstfile == FALSE)
	break;
      Log("failed to bulk read first file info...not retrying");
      //Sleep(150);
      continue;
    }
    break;
  }
  if(s!=28) {
    Log("failed to bulk read file info");
    return FALSE;
  }
  
  if(data[0]==0x0f) { //we've already read the last file
    Log("Last file.");
    return FALSE;
  }


  memset(tempname,0,14);
  memcpy(tempname,data+4,12);
  for(t=0;t<13;t++)
    if(tempname[t]==0xff)
      tempname[t]=0;



  strcpy(thisfileinfo->filename, tempname);
  thisfileinfo->filesize=*(unsigned int*)&data[20];
  thisfileinfo->filetype=FIFILE;
  if(data[18]==0x10)
    thisfileinfo->filetype=FIDIR;
	
  return TRUE;
}





gboolean ChangePartition(unsigned int partition) {

  int dummy=0x00;
  unsigned int cmd;
  
  if( (partition==1) || (partition>4)) {
    Log("refusing to change to non-filesystem partition.");
    return FALSE;
  }

  cmd=0xbf00 | partition;
  
  ControlMessageWrite(cmd,&dummy,4,SHORT_TIMEOUT); // This cmd always fails!!
  if(WaitMediaStatus()==FALSE)
    return FALSE;
  
  return TRUE;
  
}
gboolean rTrim(char * c, const char e) {
  char* d;
  if (strlen(c) == 0) return TRUE;
  d = c;
  while (*d++);
  --d;
  while (d != c) {
    if (e == *d) {
      *d = '\0';
    } else {
      return TRUE;
    }
  }
  return TRUE;

}

gboolean ChangeDirectory(const char* d) {
  
  char data[LIBUSB_PATH_MAX];
  int c;
  char directory_p[LIBUSB_PATH_MAX], *directory;
  directory = directory_p;
  strncpy(directory, d, LIBUSB_PATH_MAX - 1);
  
  //the following breaks the CD into seperate directory paths...
  //I found the complex path support in the camcorder a bit flakey.
  if((strlen(directory) > 1) && (directory[0] == '/')) {
    if(ChangeDirectory("/")==FALSE)
      return FALSE;
    //directory=directory.Mid(1,LIBUSB_PATH_MAX); //strip off leading /
    directory++;
    rTrim(directory, '/');
    
    //    directory.TrimRight("/"); //strip off trailing /
  }
  

  while((strlen(directory) > 1) &&
	(memchr(directory,'/',sizeof(char)) != NULL) ) {
    char temp[LIBUSB_PATH_MAX], *ptemp;
    strncpy(temp, directory, LIBUSB_PATH_MAX - 1);
    if (memchr(directory,'/', sizeof(char)) != NULL)
      *(char*)(memchr(directory, '/', sizeof(char))) = '\0';
    
    
    if(ChangeDirectory(temp)==FALSE) {
      Log("failed to change directory for:");
      Log(temp);
      return FALSE;
    }

    strncpy(temp, directory, LIBUSB_PATH_MAX - 1);
    directory = memchr(temp, '/', sizeof(char)) + 1;
    //ptemp[strlen(ptemp) - 1] = '\0';
    rTrim(directory, '/');
    //    directory=directory.Mid(directory.Find('/')+1,LIBUSB_PATH_MAX);
    //    directory.TrimRight("/");
  }
  
  //We're at a single level directory at this point... lets validate it
  if(strlen(directory) == 0)
    return TRUE;
  c=directory[0];
  if(! ( ((c>='a')&&(c<='z')) ||  ((c>='A')&&(c<='Z'))  || 
	 ((c>='0')&&(c<='9')) || (c=='_') || c=='/') ) {
    Log("//not liking the first character of this directory!");
    //the camcorder's CD command created directories if they're not
    //there... best to be prudent and assume it's some kind of error.
    return FALSE;
  }
  
  //validation is done... we can procede with the actual command stuff.
  memset(data,0x00,LIBUSB_PATH_MAX);
  strcpy((char *)data,directory);
  
  if(ControlMessageWrite(0xb800,(int *)data, strlen((char *)data)+1,TIMEOUT)==TRUE) {
    return TRUE;
  }
  strcpy(data, "change directory to ");
  if (strlen(data) + strlen(directory) + strlen(" failed") < LIBUSB_PATH_MAX) {
    strcat(data, directory);
    strcat(data, " failed");
    Log(data);
  }
  return FALSE;
}


void AddFileDataAsChild(file_info* temp, file_info* addeditem) {
  if (temp == NULL || addeditem == NULL)
    return;
  if (temp->number_of_children < MAX_NUMBER_OF_FILES_IN_DIRECTORY) {
    temp->children[temp->number_of_children] = addeditem;
    (temp->number_of_children)++;
    
  }
}


file_info* AddFileDataToList(file_info* temp) {
  //note: returns malloc'd data
  file_info* pointer = (file_info*)malloc(sizeof(file_info));
  int count;
  //  Log("begin AddFileDataToList");
  strcpy(pointer->filename, temp->fullpath);
  strcpy(pointer->fullpath, temp->fullpath);
  strcpy(pointer->dirpath, temp->dirpath);

  pointer->filesize = temp->filesize;
  pointer->partition = temp->partition;
  pointer->filetype = temp->filetype;
  pointer->number_of_children = temp->number_of_children;

  for (count = 0; count < temp->number_of_children; ++count) {
    pointer->children[count] = AddFileDataToList(temp->children[count]);
  }
  //Log("End AddFileDataToList");
  return pointer;


}
void FreeAllocatedFiles(file_info* temp) {
  int count;
  if (temp == NULL) return;
  for (count = 0; count < temp->number_of_children; ++count) {
    free(temp->children[count]);
  }
  temp->number_of_children = 0;

}

gboolean update_directory_listing (GtkWidget *widget,
				   GdkEvent *event,
				   gpointer data) {
  //  	HTREEITEM hItem[5],rItem;
  GtkTreeModel* model;


  if(CheckCameraOpen()==FALSE)
    return FALSE;
  EnableControls(FALSE);
  model = create_model();
  

  Log("model created.");
  FreeAllocatedFiles(root_directory);
  root_directory = NULL;
  gtk_tree_view_set_model(GTK_TREE_VIEW(m_directory_tree), GTK_TREE_MODEL(model));
  //  gtk_object_unref(GTK_TREE_MODEL(model));
  //gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_directory_tree)), GTK_SELECTION_NONE);
  EnableControls(TRUE);
  return TRUE;
}
file_info* AddToTreeStore(GtkTreeStore* treestore, GtkTreeIter* toplevel, GtkTreeIter* child, file_info* current, file_info* parent) {
  file_info* addeditem;
  gtk_tree_store_append(GTK_TREE_STORE(treestore), child, toplevel); //child is created here
  addeditem = AddFileDataToList(current);
  AddFileDataAsChild(parent, addeditem);
 
  gtk_tree_store_set(GTK_TREE_STORE(treestore), child, COL_FILENAME, current->filename, COL_POINTER, addeditem, -1);
  return addeditem;
}

GtkTreeModel* create_model() {
  GtkTreeStore* treestore = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
  GtkTreeIter rootlevel, toplevel, child;
  int t;
  file_info root_file_data, current_file_data, *addeditem = NULL;
  
  char tempstring[STRINGSIZE];
  //  EnableControls(FALSE);

  strcpy(root_file_data.filename, "/");
  root_file_data.filesize = 0;
  root_file_data.filetype = FIROOT;
  strcpy(root_file_data.fullpath, "");
  root_file_data.partition = 255;
  root_file_data.number_of_children = 0;
  

  AddToTreeStore(treestore, NULL, &rootlevel, &root_file_data, NULL);
  /*  gtk_tree_store_append(treestore, &rootlevel, NULL); //create blank entry
  root_directory = AddFileDataToList(&root_file_data);
  addeditem = root_directory;
  gtk_tree_store_set(treestore, &rootlevel, COL_FILENAME, root_file_data.filename, COL_POINTER, addeditem, -1);*/


  for (t=0; t<5; ++t) {
    if (t == 1) continue; //skip the non-filesystem partition
    
    snprintf(tempstring, STRINGSIZE-1, "p%d",t);  //partition directory name
    strcpy(current_file_data.filename, tempstring);
    current_file_data.filesize = 0;
    strcpy(current_file_data.fullpath, "/");
    strcpy(current_file_data.dirpath, "/");
    current_file_data.partition = t;
    current_file_data.filetype = FIPART;
    current_file_data.number_of_children = 0;
    strcpy(tempstring, "add filename: ");
    if (strlen(tempstring) + strlen(current_file_data.filename) < STRINGSIZE)
      strcat(tempstring, current_file_data.filename);
    //    Log("add filename:");
    Log(current_file_data.filename);

    AddToTreeStore(treestore, &rootlevel, &toplevel, &current_file_data, &root_file_data);

    if (ChangePartition(t) == FALSE) {
      Log("Couldn't change partitions");
      EnableControls(TRUE);
      return NULL;
    }
    if(ChangeDirectory("/")==FALSE) {
      Log("Couldn't change directories");
      EnableControls(TRUE);
      return NULL;
    }
    //RecursiveListing("/",addeditem,t);
    //    strcpy(tempstring, "/");
    RecursiveListing("/", addeditem, &toplevel, t, 0, treestore);
    //m_directory_tree.Expand(hItem[t],TVE_EXPAND);//hmm?
    
    //TODO: map filename data to some data in memory
    
  }

  //m_directory_tree.Expand(rItem,TVE_EXPAND);
	

  ChangePartition(0); //make sure we're back at the media partition!!!
  ChangeDirectory("/DCIM");

  //Log();
  EnableControls(TRUE);
  return GTK_TREE_MODEL(treestore);
	
}
void RecursiveListing(char* parentpath, file_info* parent, GtkTreeIter* parent_place, int partition, int level, GtkTreeStore* treestore) {
  gboolean firstitem;
  char partname[STRINGSIZE];
  file_info rData, tData, pData;
  //  const char* parentpath = parentpath_cs.text;
  int t, end;
  int number_of_hitems = 0;
  GtkTreeIter child;
  // file_info hitems[999];
  char recursedir[STRINGSIZE];
  char tempstring[STRINGSIZE];
  firstitem = TRUE;
  
  strcpy(tempstring, "RecursiveListing: ");
  if (strlen(tempstring) + strlen(parentpath) < STRINGSIZE)
    strcat(tempstring, parentpath);
  Log(tempstring);
  //Log("RecursiveListing: ");
  //Log(parentpath);
  if (level == 5) return; //recursive runaway!!! lookout!!!

  //for (t=0; t < 1000; ++t) {
    //    hItem[t].number_of_children = 0; //only parameter that would really get you in trouble
    
  // }
  //NOTE: We take 2 passes at the directory, due to the way the
  //camcorder directory listing works (doesn't lend itself to easy
  //recursion.
  
  for(t=0;t<1000;t++)  { //don't bitch. 1000 items in one directory is a lot.
    Log("Getfileinfo?");
    if(GetFileInfo(&tData,firstitem)==FALSE)
      break; //we've passed the last item.
    Log("Getfileinfo.");
    firstitem=FALSE;
    
    
    
    if( (strcmp(tData.filename,".") == 0) ||
	(strcmp(tData.filename,"..") == 0) ||
	(tData.filename[0]==0x20) ) { //0x20 == space bar?
      t--;
      continue;
    }
    
    
    
    strcpy(pData.filename,tData.filename);
    pData.filesize = tData.filesize;
    pData.filetype = tData.filetype;
    pData.number_of_children = 0;
    
    strcpy(pData.dirpath, parentpath);
    strcpy(pData.fullpath, parentpath);
    
    if (strlen(tData.filename) + strlen(pData.fullpath) < STRINGSIZE - 1)
      strcat(pData.fullpath, tData.filename);
    pData.partition=partition;
    Log(pData.fullpath);
    if(pData.filetype==FIDIR) {
      
      
      file_info* f_i = AddToTreeStore(GTK_TREE_STORE(treestore), parent_place, &child, &pData, parent);
      Log("if (pData.filetype == FIDIR)");
      strncpy (recursedir, pData.fullpath, STRINGSIZE - 2);
      strcat (recursedir, "/");
      if (ChangeDirectory(pData.fullpath) == FALSE) {
	strcpy(tempstring, "Couldn't recurse into: ");
	if (strlen(tempstring) + strlen(pData.fullpath) < STRINGSIZE)
	  strcat(tempstring, pData.fullpath);
	
	//Log("Couldn't recurse into:");
	Log(tempstring);
	return;
      }
      //      RecursiveListing(recursedir, f_i);
      //hItem[t] = m_directory_tree.InsertItem(pData->filename,0,0,parent);
      //Log("enter recursion");
      
      //RecursiveListing(NULL, NULL, NULL, 0, 0, NULL);
      Log("RecursiveListing recursion.");
      RecursiveListing(recursedir, f_i, &child, partition, level + 1, GTK_TREE_STORE(treestore));
      //Log("end recursion");
    } else {
      file_info* f_i = AddToTreeStore(treestore, parent_place, &child, &pData, parent);
      strcpy(tempstring, "Not a directory: ");
      strcat(tempstring, pData.fullpath);
      Log(tempstring);
      //Log("that point");
      
      //hItem[t] = m_directory_tree.InsertItem(pData->filename,2,2,parent);
    }
    //SetItemData(hItem[t],(DWORD)pData);;
    
  }
  end = t;

  //return; //debug... don't recurse

  /*for(t=0;t<end;t++) {
    //if(hItem[t]==0)
    //  return;
    rData=(FILE_INFO *)m_directory_tree.GetItemData(hItem[t]);
    if(rData->filetype==FIDIR)
      {
	CString recursedir=rData->fullpath + "/";
	if(m_camcorder.ChangeDirectory(rData->fullpath)==false)
	  {
	    Log("Couldn't recurse into "+rData->fullpath);
	    return;
	  }
	RecursiveListing(recursedir,hItem[t],partition, level+1);
		}
	}

  */
  
}

