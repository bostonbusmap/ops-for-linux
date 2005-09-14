#include "ops-linux.h"
typedef unsigned int DWORD;
typedef unsigned int dword;
typedef unsigned char byte;
typedef unsigned short int word;
//maybe some asserts to confirm byte sizes?
#pragma pack (push, 0)
typedef struct
{
	byte	JumpInstruction[3];
	byte	OEMID[8];
	word	BytesPerSector;
	byte	SectorsPerCluster;
	word	ReservedSectors;
	byte	FATs;
	word	RootEntries;
	word	SmallSectors;
	byte	Media;
	word	FATSize;
	word	TrackSize;
	word	Heads;
	dword	HiddenSectors;
	dword	LargeSectors;
	byte	DriveNumber;
	byte	CurrentHead;
	byte	Signature;
	dword	ID;
	byte	VolumeLabel[11];
	byte	SystemID[8];
	byte	LoadInstructions[448]; // 512-64
	word	BR_Signature; /*=AA55h*/
}  BootRecord;
#pragma pack (pop)
int GetPartitionSize(int partition, gboolean rounded) {
  int data[2];
  int y;
  BootRecord bootrec;
  int size,sectors,dummy, division_quotient;

  if(partition==1)
    return(-1); //it's not FAT, so we can't fetch it's size
  
  data[1]=512; //size of transfer
  if(partition==2)
    data[0]=512; //skip the EBR
  else
    data[0]=0; //other partitions aren't proceded with crap
  if(ControlMessageWrite(0xf100|partition,data,8,TIMEOUT)==FALSE) { // Set memory transfer filter, and size
    
    Log("failed at 0xf1, set memory type");
    return(-1);
  } 
  if(ControlMessageWrite(0xf300,&dummy,0, TIMEOUT)==FALSE) { // Initiate the memory transfer
    
    Log("failed at 0xf3, request memory");
    return(-1);
  }
  if (BUFSIZE >= 512) {
    y=Read((unsigned char *)&bootrec,512,TIMEOUT); //watch out Linux guys 512 won't work. rewrite for multiple fetches
  } else {
    int sub_y;
    int sub_count;
    division_quotient = 512 / BUFSIZE;
    for (sub_count = 0; sub_count < division_quotient; ++sub_count) {
      sub_y = Read(((unsigned char *)(&bootrec)) + BUFSIZE * sub_count,
	       BUFSIZE, TIMEOUT);
      if (sub_y != BUFSIZE) { //weirdness
	Log("ERROR: y != BUFSIZE (unable to read full boot record");
	return FALSE;
      }
      y += sub_y;
    }
    if (512 % BUFSIZE != 0) {
      y += Read(((unsigned char*)(&bootrec)) + BUFSIZE * division_quotient,
	       512 % BUFSIZE, TIMEOUT);
      
    }
    
  }

  if(y!=512) {
    Log("Couldn't retrieve partition's boot record");
    return(-1);
  }
  if(bootrec.BR_Signature!=0xaa55) {
    Log("Bad signature retrieved from partition's boot record");
    return(-1);
  }
  
  sectors=bootrec.LargeSectors+bootrec.SmallSectors;
  
  //stupid cam doesn't want to provide/send a number of sectors that isn't
  //evenly divisible by the clusters, so we have to round up the size
  if((rounded==TRUE)&&((sectors%bootrec.SectorsPerCluster)!=0)) {
    //round up the sectors to the next evenly divisible cluster count
    sectors=sectors+bootrec.SectorsPerCluster-(sectors%bootrec.SectorsPerCluster);
  }
  
  size=sectors*bootrec.BytesPerSector;
  
  return(size);
  
}




gboolean DownloadFlash(const char* filename, int partition, int size) {

  //Ok, this one's pretty ugly... The problem is that the firmware
  //doesn't want to send a file who's sector count isn't divisible
  //by the clusters-per-sector value. So if the partition size isn't
  //evenly divided by sectors divided by clusters, then we have to
  //pad it, and then throw away the padding.
  //...it could be something else, but assuming the above fixes
  //a "rounded down" bug when delivering the NO_NAME partition.
  
  unsigned char buffer[BUFSIZE];
  int data[3];
  int dummy;
  int i,x;
  int roundedsize,bufsize;

  
  
  FILE* file = fopen(filename, "wb");
  if (file == NULL) {
    Log("Trouble creating filename");
    return FALSE;
  }


  if(partition==0) { //we repurpose 0 to mean start at 3 and grab the rest of the flash
    
    partition=3;
    size=(128*1024*1024); //set bigger than the maximum we can grab
    roundedsize=size;
  }
  if(partition==1) {
    size=(32*1024*1024); //p1 appears to be a ramdisk or something 32 Meg in size.
    roundedsize=size;
  } else if(size==-1) { //they haven't requested a size, get the right size for this partition
    
    size=GetPartitionSize(partition, FALSE);
    roundedsize=GetPartitionSize(partition,TRUE);
    if(size<1)
      return FALSE;
  } else
    roundedsize=size;

  data[0]=0;
  data[1]=roundedsize;
  if(partition==2) {
    //skip the first 512 bytes (the EBR) by setting data[0]=512
    data[0]=512;
  }
  
  /*  p_ctl_progress->SetRange32(0,(roundedsize/BUFSIZE)/32);
  p_ctl_progress->SetStep(1);
  p_ctl_progress->SetPos(0);*/
  
  if(ControlMessageWrite(0xf100|partition,data,8,TIMEOUT)==FALSE) { // Set memory transfer filter, and size
    Log("failed at 0xf1, set memory type");
    return FALSE;
  }
  if(ControlMessageWrite(0xf300,&dummy,0,TIMEOUT)==FALSE) { // Initiate the memory transfer
    Log("failed at 0xf3, request memory");
    return FALSE;
  }
  
  bufsize=BUFSIZE;
  int count=0;
  for(i=0;;i++) {
    memset(buffer,0,BUFSIZE);
    x=Read((unsigned char *)buffer,bufsize,TIMEOUT);
    if(x>0) {
      //      file.WriteHuge(buffer,(DWORD)x);
      fwrite(buffer, sizeof(char), x, file);
      count+=x;
      if(count+bufsize>size) { //we gotta shrink the buffer
	bufsize=size-count;
      }
    }
    
    if(i%32==0) {
      set_progress_bar((double)count / (double)size);
      // p_ctl_progress->SendMessage(PBM_SETBARCOLOR,0,(LPARAM)RGB(0,0,220)); //clear the text
      //p_ctl_progress->StepIt();
    }
    set_bitrate(count);
    //  DoMessagePump();
    if((x<bufsize)||(count==size))
      break;
  }
  //p_ctl_progress->SetPos(0);
  if(roundedsize!=size) { //we need to bleed off the excess requested bytes.
    
    for(;;) {
      x=Read((unsigned char *)buffer,BUFSIZE,TIMEOUT); //applying leeches
      if(x<1)
	break;
    }
  }
  
  fclose(file);
  
  //CString debug; debug.Format("flash download was %d bytes",count);
  fprintf(stderr, "flash download was %d bytes\n",count);
  //  Log(debug);
  return TRUE;

}

void download_flash_start_thread(gpointer data) {
  twosome* ts = data;
  
  if(DownloadFlash((char*)(ts->a), (int)(ts->b), -1)==FALSE) {
    Log("DownloadFlash(p->filename, tempstring) failed.");
    EnableControls(TRUE);
    return FALSE;
  } else {
    Log("Success retrieving data file.");
  }
  free(ts->a);
  free(ts);


}





gboolean download_flash( GtkWidget *widget,
			 GdkEvent *event,
			 gpointer data) {
  GtkWidget *file_selection_dialog = NULL;
  char* save_filename;
  char saveto[STRINGSIZE];
  twosome* ts = NULL;
  GError* error = NULL;
  //  GtkWidget* file_selection_box = NULL;
  
  // fill in name of file for easy clicking
  //  GtkTreeModel* model;
  //GtkTreeIter iter;
  //GtkTreeSelection* selection;
  //  gpointer data;
  file_info* currently_selected_file;
  int partition_value;
  if(CheckCameraOpen()==FALSE)
    return FALSE;


  partition_value = 0; //FIXME: replace with option box like windows ops has
  
  file_selection_dialog =
    gtk_file_chooser_dialog_new("Please select a place to download the flash to",
				main_window, //meaingless except if program killed
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);
  //  gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(file_selection_dialog),currently_selected_file->filename );
  if (gtk_dialog_run(GTK_DIALOG(file_selection_dialog)) == GTK_RESPONSE_ACCEPT) {
    save_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_selection_dialog));
    strncpy(saveto, save_filename, STRINGSIZE - 1);
  } else { //user cancelled
    gtk_widget_destroy(file_selection_dialog);
    return FALSE;
  }
  gtk_widget_destroy(file_selection_dialog);
  
  if(ChangePartition(partition_value)==FALSE) {
    Log("ChangePartition(p->partition) failed.");
    return FALSE;
  }

  ts = malloc(sizeof(twosome));
  if (ts == NULL) return FALSE;
  ts->a = malloc(sizeof(char) * (strlen(saveto) + 1));
  if (ts->a == NULL) { free(ts); return FALSE; }
  strcpy(ts->a, saveto);
  ts->b = partition_value;
  /*if(ChangeDirectory(currently_selected_file->dirpath)== FALSE) {
    Log("ChangeDirectory(p->dirpath) failed.");
    return FALSE;
    }*/


  EnableControls(FALSE);

  if (!g_thread_create(download_flash_start_thread, ts, FALSE, &error)) {
    Log(error->message);
    free(ts->a);
    free(ts);
    EnableControls(TRUE);
    return FALSE;
  }

  
  
  
  
  EnableControls(TRUE);
  set_progress_bar(0.0);
  return TRUE;

  
  
}

