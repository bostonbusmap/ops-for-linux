#include "ops-linux.h"

//////////////////////////////////////////////////////////////////////////////////////////////

static gboolean FileToMemory(const char* filename, unsigned char *buffer, int maxlength) {

  // This function expects that you've already changed the parition and 
  // directory to the correct path in the camcorder.
  
  int count,x,i;
  int data;
  //CFile file;
  
  unsigned char sfilename[256];
  
  file_info fileinfo;


  if(GetAnyFileInfo(filename,&fileinfo)==FALSE) {
    Log(ERROR, "Trouble getting file size information");
    return FALSE;
  }
  
  if(fileinfo.filesize>maxlength) {
    //Log("Failed "+filename+" is larger than memory buffer.");
    Log(ERROR, "Failed, file is larger than memory buffer.");
    return(FALSE);
  }
  
  memset(sfilename,0,255);
  strcpy((char *)sfilename,filename);
  
  if(ControlMessageWrite(0xb101,(char *)sfilename,strlen((char *)sfilename)+1, TIMEOUT)==FALSE) { // SetFileName
	
    Log(ERROR, "failed at 0xb1 in FileToMemory");
    return(FALSE);
  }
  
  
  data=0x00;
  if(ControlMessageWrite(0x9301,(char*)&data,0,TIMEOUT)==FALSE) { // Request File Read
    
    Log(ERROR, "failed at 0x93 in FileToMemory");
    return FALSE;
  }
  
  
  //NOTE: we keep grabbing bytes until the camcorder is done feeding us.
  x=0;
  
  for(i=0;;i++) {
    count=Read(buffer+x,BUFSIZE,TIMEOUT);
    if(count<1)
      break;
    x=x+count;
    //    fprintf(stderr,"%d bytes done\n",x);
    if(count<BUFSIZE)
      break;
    //DoMessagePump();
  }

  return TRUE;
}

static gboolean MemoryToFile(const char* filename, char *buffer, unsigned int filesize)
{
	// This function expects that you've already changed the parition and 
	// directory.
  //Log("entering MemoryToFile");
  unsigned int x,t;
  //CFile file;
  //FILE* file = NULL;

  char sfilename[256];
  
  int bufsize;
  
  if(strlen(filename)>12) {
    Log(ERROR, "Can't upload files with long filenames (in MemoryToFile)");
    return(FALSE);
  }

  memset(sfilename, '\0', sizeof sfilename);
  strncpy(sfilename, filename, 12);
  if(ControlMessageWrite(0xb105,(char *)sfilename,strlen(sfilename)+1, TIMEOUT)==FALSE) { // SetFileName
    Log(ERROR, "failed at 0xb1 in MemoryToFile");
    return FALSE;
  }

  char udata[4];
  // It's a bisexual byte order! Arrrgh! The PDP-11 legacy lives on.
  udata[1] = filesize >> 24;
  udata[0] = filesize >> 16;
  udata[3] = filesize >>  8;
  udata[2] = filesize >>  0;
  if(ControlMessageWrite(0x9505,udata,4,TIMEOUT)== FALSE) { // Request Write
    Log(ERROR, "failed at 0x95 in MemoryToFile");
    return FALSE;
  }
  
  bufsize=BUFSIZE;
  for(t=0; t<filesize; t=t+bufsize) {
    if(t+BUFSIZE > filesize)
      bufsize=filesize-t; //last chunk... make the buffer smaller
    x=Write(buffer+t,bufsize,TIMEOUT);
    if(x<1) {
      //Log("Error writing file "+filename+" to camera");
      Log(ERROR, "Error writing file to camera");
      
      //file.Close();
      return FALSE;
    }
    //DoMessagePump();
  }
  Log(DEBUGGING, "Leaving MemoryToFile");
  return TRUE;
}



#pragma pack(push,1)
typedef struct usp_data { // file  data
  char magic0[8];         // 0x002 0x000
  char serial[16];        // 0x00a 0x008
  char zero0[112];        // 0x01a 0x018
  char challenge[128];    // 0x08a 0x088
  char response[128];     // 0x10a 0x108
  char zero1[32];         // 0x18a 0x188
  // stuff beyond here can be found at 0x80140120 in the camera
  char magic2a[12];        // 0x1aa 0x1a8, the 0x01 bytes may be compression-related
  char recycle;           // 0x1b6 0x1b4
  char magic2b;           // 0x1b7 0x1b5
  char magic2c[2];        // 0x1b8 0x1b6
  char magic3;            // 0x1ba 0x1b8
  char zero1a[4];         // 0x1bb 0x1b9
  char compression0;      // 0x1bf 0x1bd
  char compression1;      // 0x1c0 0x1be
  char magic4;            // 0x1c1 0x1bf
  char maxfiles;          // 0x1c2 0x1c0
  char softlimit;         // 0x1c3 0x1c1
  char fps;               // 0x1c4 0x1c2
  char magic5;            // 0x1c5 0x1c3
  char magic6[2];         // 0x1c6 0x1c4, 1st byte may be compression-related
  short xres;             // 0x1c8 0x1c6
  short yres;             // 0x1ca 0x1c8
  char magic7[4];         // 0x1cc 0x1ca
  char magic8;            // 0x1d0 0x1ce
  char hardlimit;         // 0x1d1 0x1cf
  char magic9[20];        // 0x1d2 0x1d0
  // end of data found starting at 0x80140120
  char zero2[1560];       // 0x1e6 0x1e4
  char magic10[2];        // 0x7fe 0x7fc
  char zero3[2];          // 0x800 0x7fe
} usp_data;               // 0x802 0x800 (offset of footer / size of struct)
#pragma pack(pop)

#pragma pack(push,1)
typedef struct usp_file {
  unsigned short header;        // 0x000, 2 bytes
  usp_data       data;          // 0x002, 2048 bytes
  unsigned short footer;        // 0x802, 2 bytes
} usp_file;                     // 0x804 bytes total
#pragma pack(pop)

static usp_file original_usp, default_usp, current_usp;

static char zero[1600];

static const char *verify_usp_data(usp_data *ud){
  if(sizeof(usp_file) != 0x804)
    return "bad struct size";

  if(memcmp(ud->magic0, "\x10\x00\x01\x00\x00\x08\x00\x00", sizeof ud->magic0))
    return "bad magic0";
  if(!memcmp(ud->serial, "Not Initialized", sizeof ud->serial)) {
    //fprintf(stderr,"serial: %s\n",ud->serial);
    Log(WARNING, "serial number \"Not Initialized\", maybe a FSP.BIN file?");
  }
  if(memcmp(ud->zero0, zero, sizeof ud->zero0))
    return "bad zero0";
  if(memcmp(ud->zero1, zero, sizeof ud->zero1))
    return "bad zero1";
  if(memcmp(ud->magic2a, "\x6c\x0f\x00\x03\x00\x01\x15\x03\x01\x01\x01\xff", sizeof ud->magic2a))
    return "bad magic2a";
  if (ud->magic2b != 0)
    return "bad magic2b";
  if (memcmp(ud->magic2c, "\x00\x00", sizeof ud->magic2c))
    return "bad magic2c";
  if(ud->magic3 != 0x14 && ud->magic3 != 0x00)
    return "bad magic3";
  if(memcmp(ud->zero1a, "\x00\x00\x00\x00", sizeof ud->zero1a))
    return "bad zero1a";
  if(ud->compression0 != 0x60 && ud->compression1 != 0x7f)
    return "unknown compression-related bytes look bad";
  if(ud->magic4 != 0)
    return "bad magic4";
  if(ud->maxfiles != 0x62) // 98 files at most -- maybe near a DVD chapter limit?
    return "bad maxfiles";
  if(ud->fps != 30 && ud->fps != 24)
    return "bad fps";
  if(ud->magic5 != 0x00)
    return "bad magic5";
  if(memcmp(ud->magic6, "\x86\x00", sizeof ud->magic6))
    return "bad magic6";

  unsigned short xres = le16_to_cpu(ud->xres);
  unsigned short yres = le16_to_cpu(ud->yres);
  if(yres & 0xf)
    return "yres not a multiple of 16";
  if(xres & 0xf)
    return "xres not a multiple of 16";
  if(xres*3u != yres*4u)
    return "resolution not a 4:3 ratio";
  if(xres > 640)
    return "resolution too high";
  if(xres < 192)
    return "resolution too low";

  if(memcmp(ud->magic7, "\x00\x00\x00\x1e", sizeof ud->magic7))
    return "bad magic7";
  if(ud->magic8 != 0x06)
    return "bad magic8";
  if(memcmp(ud->magic9, "\x20\x04\x14\x50\x78\x28\x32\x00\xb4\x01\x04\x01\x04\x00\x0a\x10\x10\x5c\x08\x0a", sizeof ud->magic9))
    return "bad magic9";
  if(memcmp(ud->zero2, zero, sizeof ud->zero2))
    return "bad zero2";
  if(!ud->magic10[0] && !ud->magic10[1]) { // zero in the FSP, which we don't want
    
    Log(WARNING, "bad magic10 (an FSP.BIN file?)");
  }
  if(memcmp(ud->zero3, zero, sizeof ud->zero3))
    return "bad zero3";

  if(ud->hardlimit && ud->hardlimit < ud->softlimit)
    return "hard limit enabled, but less than soft limit";
  if(!ud->softlimit)
    return "soft limit is zero";

  return NULL;
}

///////////////////////////////////////////////////////////////////////////////////

#if 1

static int get_usp_file(usp_file *uf) {
  // make sure the magic won't match by accident
  uf->header = cpu_to_le16(0xdead);
  uf->footer = cpu_to_le16(0xb00b);

 

  if(ChangePartition(3)==FALSE) {
    return FALSE;
  }
  if(ChangeDirectory("/")== FALSE) {
    return FALSE;
  }
  
  if(FileToMemory("USP.BIN", (char*)uf, sizeof *uf)==FALSE) {
    Log(ERROR, "FileToMemory failed in get_usp_file");
  }
      
  //fprintf(stderr, "Got 0x%03x (%d) bytes\n", count, count);  

  return uf->header==cpu_to_le16(0xfaac) && uf->footer==cpu_to_le16(0xfaac);
}

#else

static int get_usp_file(usp_file *uf){
  FILE *fp = fopen("cam.BIN","rb");
  if(!fp) return 0;
  int rc = fread(uf, sizeof *uf, 1, fp);
  fclose(fp);
  return rc==1 && uf->header==cpu_to_le16(0xfaac) && uf->footer==cpu_to_le16(0xfaac);
}

#endif


#if 1

static int put_usp_file(usp_file *uf){
  

  if(ChangePartition(3)==FALSE) {
    return FALSE;
  }
  if(ChangeDirectory("/")== FALSE) {
    return FALSE;
  }

  if(MemoryToFile("USP.BIN", (char*)uf, sizeof *uf)==FALSE) {
    Log(ERROR, "FileToMemory failed in get_usp_file");
  }

  return TRUE;
}

#else

static int put_usp_file(usp_file *uf){
  FILE *fp = fopen("cam.BIN","wb");
  if(!fp) return 1;
  int rc = fwrite(uf, sizeof *uf, 1, fp);
  fclose(fp);
  return rc==1;
}

#endif

/////////////////////////////////////////////////////////////////////////////////

static GtkWidget *window;

// I keep forgetting
#define button_default button_defaults
#define settings_defaults settings_default

//static GtkScale *hard_scale, *soft_scale, *size_scale;
static GtkWidget *hard_scale, *soft_scale, *size_scale;

static GtkAdjustment *hard_adj, *soft_adj, *size_adj;
//static GtkObject *hard_adj, *soft_adj, *size_adj;

static GtkWidget *button30, *button24, *checkbox, *checkbox_recycle;

static GtkWidget *button_revert = NULL, *button_write = NULL, *button_cancel = NULL, *button_defaults = NULL;

static void set_button_states(void){
  if (button_default) { //chances are all or none exist
    gtk_widget_set_sensitive(button_default, !!memcmp(&current_usp, &default_usp, sizeof current_usp)); 
    gtk_widget_set_sensitive(button_revert, !!memcmp(&current_usp, &original_usp, sizeof current_usp)); 
    gtk_widget_set_sensitive(button_write, !!memcmp(&current_usp, &original_usp, sizeof current_usp)); 
    gtk_widget_set_sensitive(hard_scale, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)));
    //    gtk_widget_set_sensitive(hard_scale, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox_recycle)));
  }
}

static gboolean settings_defaults(GtkWidget *widget, GdkEvent *event, gpointer data){
  current_usp.data.xres = cpu_to_le16(320);
  current_usp.data.yres = cpu_to_le16(240);
  size_adj->value = 320;
  g_signal_emit_by_name(G_OBJECT(size_adj), "changed");

  current_usp.data.softlimit = 20;
  soft_adj->value = 20;
  g_signal_emit_by_name(G_OBJECT(soft_adj), "changed");

  current_usp.data.hardlimit = 25;
  hard_adj->value = 25;
  g_signal_emit_by_name(G_OBJECT(hard_adj), "changed");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), TRUE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox_recycle), FALSE);

  current_usp.data.fps = 30;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button30), TRUE);

  set_button_states();
  return FALSE;
}

static gboolean settings_revert(GtkWidget *widget, GdkEvent *event, gpointer data){
  memcpy(&current_usp, &original_usp, sizeof current_usp);

  size_adj->value = le16_to_cpu(current_usp.data.xres);
  g_signal_emit_by_name(G_OBJECT(size_adj), "changed");

  soft_adj->value = current_usp.data.softlimit;
  g_signal_emit_by_name(G_OBJECT(soft_adj), "changed");

  unsigned hard = current_usp.data.hardlimit;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), !!hard);
  unsigned recycle_status = current_usp.data.recycle;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox_recycle), !!recycle_status);
  hard_adj->value = hard ? hard : soft_adj->value;
  g_signal_emit_by_name(G_OBJECT(hard_adj), "changed");

  GtkWidget *tog = current_usp.data.fps==30 ? button30 : button24;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tog), TRUE);

  set_button_states();
  return FALSE;
}

static gboolean settings_write(GtkWidget *widget, GdkEvent *event, gpointer data){
  Log(NOTICE, "Writing data...");
  if(put_usp_file(&current_usp))
    Log(NOTICE, "Data written.");
  else
    Log(WARNING, "Data NOT written.");
  PowerdownCamcorder();
  gtk_main_quit();
  return FALSE;
}

static gboolean settings_cancel(GtkWidget *widget, GdkEvent *event, gpointer data){
  gtk_widget_destroy(window);
  return FALSE;
}


// be double sure we don't round down when converting from float
#define ROUND(x) ((unsigned)((x)+0.5))

static void settings_size(GtkAdjustment *adj, GtkWidget *foo){
  gdouble val = gtk_adjustment_get_value(adj);
//  fprintf(stderr, "%f\n", gtk_adjustment_get_value(adj));
  unsigned u = ROUND(val/64.0);
  current_usp.data.xres = cpu_to_le16(u*64);
  current_usp.data.yres = cpu_to_le16(u*48);
  size_adj->value = u*64;
  g_signal_emit_by_name(G_OBJECT(size_adj), "changed");
  set_button_states();
}

static void settings_soft(GtkAdjustment *adj, GtkWidget *foo){
  gdouble val = gtk_adjustment_get_value(adj);
//  fprintf(stderr, "%f\n", gtk_adjustment_get_value(adj));
  if(val > gtk_adjustment_get_value(hard_adj)){
    hard_adj->value = val;
    g_signal_emit_by_name(G_OBJECT(hard_adj), "changed");
    current_usp.data.hardlimit = ROUND(val); // redundant?
  }
  current_usp.data.softlimit = ROUND(val);
  set_button_states();
}

static void settings_hard(GtkAdjustment *adj, GtkWidget *foo){
  gdouble val = gtk_adjustment_get_value(adj);
//  fprintf(stderr, "%f\n", gtk_adjustment_get_value(adj));
  if(val < gtk_adjustment_get_value(soft_adj)){
    soft_adj->value = val;
    g_signal_emit_by_name(G_OBJECT(soft_adj), "changed");
    current_usp.data.softlimit = ROUND(val);  // redundant?
  }
  current_usp.data.hardlimit = ROUND(val);
  set_button_states();
}

static void do_checkbox(GtkWidget *widget, gpointer data){
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))){
    current_usp.data.hardlimit = ROUND(gtk_adjustment_get_value(hard_adj));
  }else{
    current_usp.data.hardlimit = 0;
  }
  set_button_states();
}

static void do_checkbox_recycle(GtkWidget *widget, gpointer data){
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))){
    current_usp.data.recycle = 3;
  }else{
    current_usp.data.recycle = 0;
  }
  set_button_states();
}


static void do_fps(GtkWidget *widget, gpointer data){
  current_usp.data.fps = (long)data;
  set_button_states();
}



static gboolean SettingsDialog(usp_data *ud){
  GtkTooltips *tooltips = gtk_tooltips_new();
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  GtkWidget *box1 = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), box1);
  gtk_widget_show (box1);

  GtkWidget *table = gtk_table_new(3,3,FALSE);
  gtk_box_pack_start(GTK_BOX(box1), table, TRUE, TRUE, 0);
  gtk_widget_show(table);

  size_scale = gtk_hscale_new_with_range(192,640,64);
  soft_scale = gtk_hscale_new_with_range(1,255,1);
  hard_scale = gtk_hscale_new_with_range(1,255,1);
#if 0
  gtk_scale_set_digits(size_scale, 0);
  gtk_scale_set_digits(soft_scale, 0);
  gtk_scale_set_digits(hard_scale, 0);
  gtk_scale_set_draw_value(GTK_SCALE(size_scale), FALSE);
  gtk_scale_set_draw_value(GTK_SCALE(soft_scale), FALSE);
  gtk_scale_set_draw_value(GTK_SCALE(hard_scale), FALSE);
#endif
  gtk_scale_set_value_pos(GTK_SCALE(size_scale), GTK_POS_LEFT);
  gtk_scale_set_value_pos(GTK_SCALE(soft_scale), GTK_POS_LEFT);
  gtk_scale_set_value_pos(GTK_SCALE(hard_scale), GTK_POS_LEFT);
  size_adj = gtk_range_get_adjustment(GTK_RANGE(size_scale));
  soft_adj = gtk_range_get_adjustment(GTK_RANGE(soft_scale));
  hard_adj = gtk_range_get_adjustment(GTK_RANGE(hard_scale));
  size_adj->value = le16_to_cpu(current_usp.data.xres);
  soft_adj->value = current_usp.data.softlimit;
  hard_adj->value = current_usp.data.hardlimit;
  g_signal_emit_by_name(G_OBJECT(size_adj), "changed");
  g_signal_emit_by_name(G_OBJECT(soft_adj), "changed");
  g_signal_emit_by_name(G_OBJECT(hard_adj), "changed");
  // gtk_signal_connect ?
  g_signal_connect (G_OBJECT(size_adj), "value_changed", G_CALLBACK(settings_size), (gpointer)NULL);
  g_signal_connect (G_OBJECT(soft_adj), "value_changed", G_CALLBACK(settings_soft), (gpointer)NULL);
  g_signal_connect (G_OBJECT(hard_adj), "value_changed", G_CALLBACK(settings_hard), (gpointer)NULL);
  gtk_box_pack_start(GTK_BOX(box1), size_scale, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box1), soft_scale, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box1), hard_scale, TRUE, TRUE, 0);
  gtk_widget_show(size_scale);
  gtk_widget_show(soft_scale);
  gtk_widget_show(hard_scale);
  gtk_tooltips_set_tip(tooltips, size_scale, "X resolution (Y is 3/4 of X)", NULL);
  gtk_tooltips_set_tip(tooltips, soft_scale, "the soft limit in minutes, beyond which you can not start a new video (low values reduce compression artifacts)", NULL);
  gtk_tooltips_set_tip(tooltips, hard_scale, "the hard limit in minutes, beyond which you can not continue recording", NULL);

  checkbox = gtk_check_button_new_with_mnemonic("_enable hard limit");
  g_signal_connect (G_OBJECT(checkbox), "toggled", G_CALLBACK(do_checkbox), (gpointer)NULL);
  gtk_box_pack_start(GTK_BOX (box1), checkbox, TRUE, TRUE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), !!ud->hardlimit);
  gtk_widget_show(checkbox);
  gtk_tooltips_set_tip(tooltips, checkbox, "uncheck to disable the hard limit (bad idea)", NULL);

  checkbox_recycle = gtk_check_button_new_with_mnemonic("_enable ready to recycle");
  g_signal_connect (G_OBJECT(checkbox_recycle), "toggled", G_CALLBACK(do_checkbox_recycle), (gpointer)NULL);
  gtk_box_pack_start(GTK_BOX (box1), checkbox_recycle, TRUE, TRUE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox_recycle), !!ud->recycle);
  gtk_widget_show(checkbox_recycle);
  gtk_tooltips_set_tip(tooltips, checkbox_recycle, "check to trigger camera 'ready to recycle' flag", NULL);



  button24 = gtk_radio_button_new_with_label(NULL, "24 frames per second");
  g_signal_connect (G_OBJECT(button24), "toggled", G_CALLBACK(do_fps), (gpointer)24);
  gtk_box_pack_start(GTK_BOX (box1), button24, TRUE, TRUE, 0);
  if(ud->fps == 24)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button24), TRUE);
  gtk_widget_show(button24);
  gtk_tooltips_set_tip(tooltips, button24, "a slow rate that saves space and transfer time", NULL);

  button30 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button24), "30 frames per second");
  g_signal_connect (G_OBJECT(button30), "toggled", G_CALLBACK(do_fps), (gpointer)30);
  gtk_box_pack_start (GTK_BOX (box1), button30, TRUE, TRUE, 0);
  gtk_widget_show (button30);
  if(ud->fps == 30)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button30), TRUE);
  gtk_tooltips_set_tip(tooltips, button30, "a rate appropriate for conversion to TV or DVD", NULL);

  GtkWidget *box2 = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (box1), box2);
  gtk_container_set_border_width (GTK_CONTAINER (box2), 10);
  gtk_widget_show (box2);

  button_defaults = gtk_button_new_with_mnemonic("_Defaults");
  gtk_signal_connect (GTK_OBJECT (button_defaults), "clicked", 
		      GTK_SIGNAL_FUNC(settings_defaults), NULL);
  gtk_box_pack_start(GTK_BOX(box2), button_defaults, TRUE, TRUE, 0);
  gtk_widget_show(button_defaults);
  gtk_tooltips_set_tip(tooltips, button_defaults, "choose factory defaults", NULL);

  button_write = gtk_button_new_with_mnemonic("_Write");
  gtk_signal_connect (GTK_OBJECT (button_write), "clicked", 
		      GTK_SIGNAL_FUNC(settings_write), NULL);
  gtk_box_pack_start (GTK_BOX (box2), button_write, TRUE, TRUE, 0);
  gtk_widget_show (button_write);
  gtk_tooltips_set_tip(tooltips, button_write, "write to the camera and turn it off", NULL);

  button_revert = gtk_button_new_with_mnemonic("_Revert");
  gtk_signal_connect (GTK_OBJECT (button_revert), "clicked", 
		      GTK_SIGNAL_FUNC(settings_revert), NULL);
  gtk_box_pack_start (GTK_BOX (box2), button_revert, TRUE, TRUE, 0);
  gtk_widget_show (button_revert);
  gtk_tooltips_set_tip(tooltips, button_revert, "undo all changes recently made", NULL);

  button_cancel = gtk_button_new_with_mnemonic("_Cancel");
  gtk_signal_connect (GTK_OBJECT (button_cancel), "clicked", 
		      GTK_SIGNAL_FUNC(settings_cancel), NULL);
  gtk_box_pack_start (GTK_BOX (box2), button_cancel, TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS (button_cancel, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button_cancel);
  gtk_widget_show (button_cancel);
  gtk_tooltips_set_tip(tooltips, button_cancel, "quit dialog without modifying the camera", NULL);


  set_button_states();
  gtk_widget_show_all (window);
  return TRUE;
}

gboolean change_camera_settings(GtkWidget *widget, GdkEvent *event, gpointer data){
  if (CheckCameraOpen() == FALSE)
    return FALSE;
  if(sizeof original_usp != 0x804 || !get_usp_file(&original_usp)){
    return FALSE; // is anybody checking this???
  }
  const char *msg = verify_usp_data(&original_usp.data);
  if(msg){
    Log(ERROR, "%s", msg);
    return FALSE;
  }

  memcpy(&current_usp, &original_usp, sizeof original_usp);
  memcpy(&default_usp, &original_usp, sizeof original_usp);
  default_usp.data.fps = 30;
  default_usp.data.hardlimit = 25;
  default_usp.data.softlimit = 20;
  default_usp.data.xres = cpu_to_le16(320);
  default_usp.data.yres = cpu_to_le16(240);

  if(!SettingsDialog(&original_usp.data))
    return FALSE;
  Log(DEBUGGING, "called SettingsDialog and returned");
  return TRUE; // success
}


//////////////////////////////////////////////////////////////////////////////////////////////

