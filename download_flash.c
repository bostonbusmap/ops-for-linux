#include "ops-linux.h"


#pragma pack (push, 1)
typedef struct
{
	u8	JumpInstruction[3];
	u8	OEMID[8];
	u16	BytesPerSector;
	u8	SectorsPerCluster;
	u16	ReservedSectors;
	u8	FATs;
	u16	RootEntries;
	u16	SmallSectors;
	u8	Media;
	u16	FATSize;
	u16	TrackSize;
	u16	Heads;
	u32	HiddenSectors;
	u32	LargeSectors;
	u8	DriveNumber;
	u8	CurrentHead;
	u8	Signature;
	u32	ID;
	u8	VolumeLabel[11];
	u8	SystemID[8];
	u8	LoadInstructions[448]; // 512-64
	u16	BR_Signature; /*=AA55h*/
}  BootRecord;
#pragma pack (pop)


void GetFirmwareRevision(char* firmware) {
  u8 version[4];
  
  u32 data = cpu_to_le32(0x201); //request version
  if (ControlMessageWrite(0xfe01, (char*)&data, 4, TIMEOUT) == FALSE) {
    Log(ERROR, "failed at 0xfe, retrieving firmware revision, in GetFirmwareREvision");
    strcpy(firmware, "Unknown");
    return;
  }
  ControlMessageRead(0xff01, (char*)version, 4, TIMEOUT);
  
  snprintf(firmware, STRINGSIZE, "%02x.%02x",version[1],version[0]);
  //WARNING: possible endian trouble here

}

static int GetPartitionSize(int partition, gboolean rounded) {
  int data[2];
  int y = 0;
  BootRecord bootrec;
  int size,sectors,dummy, division_quotient;

  if(partition==1)
    return(-1); //it's not FAT, so we can't fetch it's size
  
  data[1]=512; //size of transfer
  if(partition==2)
    data[0]=512; //skip the EBR
  else
    data[0]=0; //other partitions aren't proceded with crap
  
  if(ControlMessageWrite(0xf100|partition,(char*)data,8,TIMEOUT)==FALSE) { // Set memory transfer filter, and size
    
    Log(ERROR, "failed at 0xf1, set memory type, in GetPartitionSize");
    return(-1);
  } 
  if(ControlMessageWrite(0xf300,(char*)&dummy,0, TIMEOUT)==FALSE) { // Initiate the memory transfer
    
    Log(ERROR, "failed at 0xf3, request memory, in GetPartitionSize");
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
	Log(ERROR, "y != BUFSIZE (unable to read full boot record), in GetPartitionSize");
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
    Log(ERROR, "Couldn't retrieve partition's boot record, in GetPartitionSize");
    return(-1);
  }
  if(bootrec.BR_Signature != cpu_to_le16(0xaa55)) {
    Log(ERROR, "Bad signature retrieved from partition's boot record: %04x (in GetParititionSize)", bootrec.BR_Signature);
    return(-1);
  }
  
  sectors = le16_to_cpu(bootrec.SmallSectors);
  if(!sectors) sectors = le32_to_cpu(bootrec.LargeSectors);
  
  //stupid cam doesn't want to provide/send a number of sectors that isn't
  //evenly divisible by the clusters, so we have to round up the size
  if((rounded==TRUE) && ((sectors%bootrec.SectorsPerCluster)!=0)) {
    //round up the sectors to the next evenly divisible cluster count
    sectors = sectors + bootrec.SectorsPerCluster - (sectors%bootrec.SectorsPerCluster);
  }
  
  size = sectors * le16_to_cpu(bootrec.BytesPerSector);
  
  return size;
}




static gboolean DownloadFlash(const char* filename, int partition, int size) {

  //Ok, this one's pretty ugly... The problem is that the firmware
  //doesn't want to send a file who's sector count isn't divisible
  //by the clusters-per-sector value. So if the partition size isn't
  //evenly divided by sectors divided by clusters, then we have to
  //pad it, and then throw away the padding.
  //...it could be something else, but assuming the above fixes
  //a "rounded down" bug when delivering the NO_NAME partition.
  
  u8 buffer[BUFSIZE];
  u32 data[3];
  u32 dummy;
  u32 i,x;
  u32 roundedsize,bufsize;
  gboolean wholeimage = FALSE;

  
  FILE* file = fopen(filename, "wb");
  if (file == NULL) {
    Log(ERROR, "Trouble creating filename, in DownloadFlash");
    return FALSE;
  }

  if(partition==0) { //we repurpose 0 to mean start at 3 and grab the rest of the flash
    
    partition=3;
    size=(128*1024*1024); //set bigger than the maximum we can grab
    roundedsize=size;
    wholeimage = TRUE;
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
  data[1]=cpu_to_le32(roundedsize);
  if(partition==2) {
    //skip the first 512 bytes (the EBR) by setting data[0]=512
    data[0]=cpu_to_le32(512);
  }
  
  /*  p_ctl_progress->SetRange32(0,(roundedsize/BUFSIZE)/32);
  p_ctl_progress->SetStep(1);
  p_ctl_progress->SetPos(0);*/
  
  if(ControlMessageWrite(0xf100|partition,(char*)data,8,TIMEOUT)==FALSE) { // Set memory transfer filter, and size
    Log(ERROR, "failed at 0xf1, set memory type, in DownloadFlash");
    return FALSE;
  }
  if (wholeimage == TRUE) {
    char frev[STRINGSIZE];
    GetFirmwareRevision(frev);
    if (strcmp(frev, "03.40") == 0)
      Monitor("wl 8012fee4 0x05");
    else if (strcmp(frev, "03.62") == 0)
      Monitor("wl 80130098 0x05");
    else {
      Log(WARNING, "Whole image download impossible. Can't find firmware revision");
      Log(WARNING, "Flash image will begin at first filesystem");
    }
  }

  if(ControlMessageWrite(0xf300,(char*)&dummy,0,TIMEOUT)==FALSE) { // Initiate the memory transfer
    Log(ERROR, "failed at 0xf3, request memory, in GetPartitionSize");
    return FALSE;
  }
  
  bufsize=BUFSIZE;
  int count=0;
  for(i=0;;i++) {
    memset(buffer,0,BUFSIZE);
    x=Read(buffer,bufsize,TIMEOUT);
    if(x>0) {
      //      file.WriteHuge(buffer,(DWORD)x);
      fwrite(buffer, 1, x, file);
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
      x=Read(buffer,BUFSIZE,TIMEOUT); //applying leeches
      if(x<1)
	break;
    }
  }
  
  fclose(file);
  
  //CString debug; debug.Format("flash download was %d bytes",count);
  Log(NOTICE, "flash download was %d bytes",count);
  //  Log(debug);
  return TRUE;
}


static void download_flash_start_thread(gpointer data) {
  twosome* ts = data;
  
  if(DownloadFlash((char*)(ts->a), (int)(ts->b), -1)==FALSE) {
    Log(ERROR, "DownloadFlash(%s, %08x, -1) failed.", (char*)(ts->a), (int)(ts->b));
    EnableControls(TRUE);
    free(ts->a);
    free(ts);
    return;
  } else {
    Log(NOTICE, "Success retrieving data file.");
  }
  free(ts->a);
  free(ts);
}



gboolean download_flash( GtkWidget *widget,
			 GdkEvent *event,
			 gpointer data) {
  GtkWidget *file_selection_dialog = NULL;
  char* filename_malloc;
  
  twosome* ts = NULL;
  GError* error = NULL;
  //  GtkWidget* file_selection_box = NULL;
  
  // fill in name of file for easy clicking
  //  GtkTreeModel* model;
  //GtkTreeIter iter;
  //GtkTreeSelection* selection;
  //  gpointer data;
  //  file_info* currently_selected_file;
  int partition_value;
  if(CheckCameraOpen()==FALSE)
    return FALSE;


  //partition_value = 0; //FIXME: replace with option box like windows ops has
  partition_value = text_option_box(6,
				    "Choose which part of flash to download from",
				    "All accessable flash",
				    "Unknown partition",
				    "p0, NO_NAME partition",
				    "p2, ResourcesA partition",
				    "p3, ResourcesB partition",
				    "p4, ResourcesC partition");
  if (partition_value < 0) {
    Log(ERROR,"Invalid partition selected");
    return FALSE;
  }
  // the translation from index number to a real partition number is done in
  //DownloadFlash()
  filename_malloc = get_download_filename(NULL);

  
  if(ChangePartition(0)==FALSE) {
    Log(ERROR, "ChangePartition(%d) failed.", partition_value);
    return FALSE;
  }

  ts = malloc(sizeof(twosome));
  if (ts == NULL) return FALSE;
  ts->a = filename_malloc;
  ts->b = (void*)partition_value;
  /*if(ChangeDirectory(currently_selected_file->dirpath)== FALSE) {
    Log("ChangeDirectory(p->dirpath) failed.");
    return FALSE;
    }*/


  EnableControls(FALSE);

  if (!g_thread_create((GThreadFunc)download_flash_start_thread, ts, FALSE, &error)) {
    Log(ERROR, "g_thread says: %s", error->message);
    free(ts->a);
    free(ts);
    EnableControls(TRUE);
    return FALSE;
  }

  EnableControls(TRUE);
  set_progress_bar(0.0);
  return TRUE;
}
