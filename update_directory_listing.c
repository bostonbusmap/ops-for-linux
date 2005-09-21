#include "ops-linux.h"

static gboolean WaitMediaStatus(void) {
  //	Another experimental function
  unsigned int cmd;
  char data[10];
  
  return TRUE;	//can't seem to get this to ensure media is available
					//maybe it does something else
  cmd=0xb600;
  if(ControlMessageRead(cmd,data,10,TIMEOUT)) {
    return TRUE;
  }
  
  return FALSE;

}

typedef struct cvs_dir_entry {
  unsigned char  unknown1[4];  //  0  if 1st byte 0x0f, no more files
  char           filename[12]; //  4  MS-DOS style, but with explicit '.'
  unsigned char  filetype;     // 16  0x00==regular, 0x77==directory
  unsigned char  unknown2;     // 17
  unsigned char  fileattr;     // 18  0x20==regular, 0x10==directory
  unsigned char  unknown3;     // 19
  unsigned       filesize;     // 20  little-endian
  unsigned short modtime;      // 24  crazy DOS time
  unsigned short moddate;      // 26  crazy DOS date
                               // 28  (this is a 28-byte struct)
} cvs_dir_entry;


// this function assumes that you've already changed to the right partition
// and directory
gboolean GetAnyFileInfo(const char* filename, file_info *thisfileinfo) {
  cvs_dir_entry data;

  Log("GetAnyFileInfo start");
  memset(&data, 0, sizeof data);
  
  // first we set the filename
  if(ControlMessageWrite(0xb101, filename, strlen(filename)+1, TIMEOUT)==FALSE) { //SetFileName
    Log("failed to set filename");
    return FALSE;
  }
  ///  //b901 doesn't work for some reason... defaulting to bc00 for the time being
  //nevermind...
  ControlMessageWrite(0xb901, (const char *)&data, 0, TIMEOUT);
  if(Read((char*)&data,28,TIMEOUT)<28) {
    Log("failed to retrieve file information");
    return FALSE;
  }
  
  Log("success in Read in GetAnyFileInfo");

  // filename may have trailing 0xff garbage
  char *cp = memchr(data.filename, '\xff', 12);
  if(!cp)
    cp = memchr(data.filename, '\0', 12);
  if(!cp)
    cp = data.filename + 12;

  size_t len = cp - data.filename;

  memcpy(thisfileinfo->filename, data.filename, len);
  data.filename[len] = '\0';

  thisfileinfo->filesize = le32_to_cpu(data.filesize);

  thisfileinfo->filetype = FIFILE;
  if(data.fileattr==0x10)
    thisfileinfo->filetype = FIDIR;

  Log("success in GetAnyFileInfo");
  return TRUE;
}


gboolean GetFileInfo(file_info* thisfileinfo, gboolean isfirstfile) {
  cvs_dir_entry data;
  int dummy,command,s,t;
  memset(&data, 0, sizeof data);

  if(isfirstfile==TRUE)
    command=0xbc00; //first file

  if(isfirstfile==FALSE)
    command=0xbd00; //next file
  
  //sometimes the first-file read fails after a partition change
  for(t=0;t<2;t++) {
    if(ControlMessageWrite(command,(char*)&dummy,0,TIMEOUT)==FALSE) {
      Log("failed at 0xbc (requesting first file info).");
      return FALSE;
    }
    if(((s=Read((char*)&data,28,TIMEOUT)))<28) {
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
  
  if(data.unknown1[0]==0x0f) { //we've already read the last file
    Log("Last file.");
    return FALSE;
  }
  {
    // filename can, oddly, be padded with 0xff
    char *cp = memchr(data.filename, 0xff, sizeof data.filename);
    if(cp)
      *cp = '\0';
    memcpy(thisfileinfo->filename, data.filename, sizeof data.filename);
    thisfileinfo->filename[sizeof data.filename] = '\0';
  }

  thisfileinfo->filesize = le32_to_cpu(data.filesize);

  switch(data.fileattr){
  case 0x10:
    thisfileinfo->filetype = FIDIR;
    break;
  default:
    Log("unknown file type");
    fprintf(stderr, "type 0x%02x attr 0x%02x\n", data.filetype, data.fileattr);
  case 0x20:
    thisfileinfo->filetype = FIFILE;
    break;
  }
  
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
  
  ControlMessageWrite(cmd,(char*)&dummy,4,SHORT_TIMEOUT); // This cmd always fails!!
  if(WaitMediaStatus()==FALSE)
    return FALSE;
  
  return TRUE;
  
}


static gboolean rTrim(char * c, const char e) {
  //erase all chars of value e to the right of c
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
  
  printf("ChangeDirectory \"%s\"\n", d);
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
  

  while( strlen(directory)>1 && directory[0]=='/' ) { //FIXME
    char temp[LIBUSB_PATH_MAX];
    strncpy(temp, directory, LIBUSB_PATH_MAX - 1);
    if (directory[0]=='/')
      directory[0] = '\0';    
    
    if(ChangeDirectory(temp)==FALSE) {
      Log("failed to change directory for:");
      Log(temp);
      return FALSE;
    }

    strncpy(temp, directory, LIBUSB_PATH_MAX - 1);
    directory = temp + 1;
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
  memset(&data,0x00,LIBUSB_PATH_MAX);
  strcpy(data,directory);
  
  if(ControlMessageWrite(0xb800,data, strlen(data)+1,TIMEOUT)==TRUE) {
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


static void AddFileDataAsChild(file_info* temp, file_info* addeditem) {
  if (temp == NULL || addeditem == NULL)
    return;
  if (temp->number_of_children < MAX_NUMBER_OF_FILES_IN_DIRECTORY) {
    temp->children[temp->number_of_children] = addeditem;
    (temp->number_of_children)++;
    
  }
}


static file_info* AddFileDataToList(file_info* temp) {
  //note: returns malloc'd data
  file_info* pointer = malloc(sizeof(file_info));
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

static void FreeAllocatedFiles(file_info* temp) {
  int count;
  if (temp == NULL) return;
  for (count = 0; count < temp->number_of_children; ++count) {
    free(temp->children[count]);
  }
  temp->number_of_children = 0;

}

static file_info* AddToTreeStore(GtkTreeStore* treestore, GtkTreeIter* toplevel, GtkTreeIter* child, file_info* current, file_info* parent) {
  file_info* addeditem;
  GdkPixbuf* pixbuf = NULL;
  gtk_tree_store_append(GTK_TREE_STORE(treestore), child, toplevel); //child is created here
  addeditem = AddFileDataToList(current);
  AddFileDataAsChild(parent, addeditem);
  
  switch (addeditem->filetype) {
  case FIFILE:
    {
      pixbuf = icon_binfile;
      char *cp = strchr(addeditem->filename, '.');
      if(!cp) break;
      cp++;
      if(!strcasecmp(cp,"avi")) pixbuf = icon_avifile;
      else
      if(!strcasecmp(cp,"jpg")) pixbuf = icon_jpgfile;
      else
      if(!strcasecmp(cp,"txt")) pixbuf = icon_txtfile;
      else
      if(!strcasecmp(cp,"wav")) pixbuf = icon_wavfile;
      else
      if(!strcasecmp(cp,"zbm")) pixbuf = icon_zbmfile;
      else
      if(!strcasecmp(cp,"avi")) pixbuf = icon_avifile;
    }
    break;
  case FIDIR:
    pixbuf = icon_blankdir;
    break;
  case FIPART:
    pixbuf = icon_blankdir;
    break;
  case FIROOT:
    pixbuf = icon_blankdir;
    break;
  default:
    break;
  };
    
  gtk_tree_store_set(GTK_TREE_STORE(treestore), child, COL_ICON, pixbuf, COL_FILENAME, current->filename, COL_POINTER, addeditem, -1);
  return addeditem;
}



static void RecursiveListing(char* parentpath, file_info* parent, GtkTreeIter* parent_place, int partition, int level, GtkTreeStore* treestore) {
  gboolean firstitem;
  file_info tData, pData;
  int count;
  //  const char* parentpath = parentpath_cs.text;
  int t, end;
  GtkTreeIter child;
  // file_info hitems[999];
  char recursedir[STRINGSIZE];
  char tempstring[STRINGSIZE];
  file_info* f_i = NULL, tempfileinfo;
  int total_getfileinfo_calls = 0;
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
    if(GetFileInfo(&tData, firstitem)==FALSE)
      break; //we've passed the last item.
    ++total_getfileinfo_calls;
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
      
      
    f_i = AddToTreeStore(GTK_TREE_STORE(treestore), parent_place, &child, &pData, parent);
    //from here on in f_i and pData should contain the same info (at different memory locations)
    //also, AddToTreeStore stores data elsewhere so don't delete f_i
    if(pData.filetype==FIDIR) {
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
      //      strncpy(tempstring, f_i->dirpath, STRINGSIZE - 2); //really hoping dirpath is this dir
      //      strcat(tempstring, "/");
      if (ChangeDirectory(pData.dirpath) == FALSE) {
	Log("PROBLEM: couldn't change back to current directory after recursion");
	return;
      }
      if (firstitem == TRUE) firstitem = FALSE;
      GetFileInfo(&pData, TRUE);
      for (count = 1; count < total_getfileinfo_calls; ++count) {
	GetFileInfo(&pData, FALSE);
      }
      //Log("end recursion");
    } else {
	//AddToTreeStore(treestore, parent_place, &child, &pData, parent);
      strcpy(tempstring, "Not a directory: ");
      strcat(tempstring, pData.fullpath);
      Log(tempstring);
      //Log("that point");
      
      //hItem[t] = m_directory_tree.InsertItem(pData->filename,2,2,parent);
    }
    //SetItemData(hItem[t],(DWORD)pData);;
    
  }
  //end = t;

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

static GtkTreeModel* create_model(void) {
  GtkTreeStore* treestore = gtk_tree_store_new(3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_POINTER);
  GtkTreeIter rootlevel, toplevel;
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
  gtk_tree_view_expand_all(GTK_TREE_VIEW(m_directory_tree)); 
  EnableControls(TRUE);
  return TRUE;
}
