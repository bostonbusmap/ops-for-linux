#ifndef OPS_LINUX
#define OPS_LINUX

#include "endian-define.h"

/////////////////////////////////////////////////////////


#include "io/io.h"
#include "widgets/widgets.h"
#include "log.h"

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <usb.h>
#include <stdint.h>

//////////////////////////////////////////////////////////

/*  definitions  */


#define COL_FILENAME 1
#define COL_POINTER 2
#define COL_ICON 0


#define FIFILE	0
#define FIDIR	1
#define FIPART	2
#define FIROOT	3

#define MAX_NUMBER_OF_FILES_IN_DIRECTORY 1000

#ifndef LIBUSB_PATH_MAX  //libusb may define it already
#define LIBUSB_PATH_MAX 4096
#endif

#ifdef _WIN32
#define BUFSIZE 4096
#else
#define BUFSIZE 64 //it's a linux thing... :(
#endif

#define NEWLINE "\n"

//#define DEBUG


/////////////////////////////////////////////////////////////////


typedef struct {
  char text[STRINGSIZE];
} constant_string;  //mostly useless

typedef struct file_info { //each one of these is at least 4000 bytes... :(
  char filename[STRINGSIZE];
  int filesize;
  char fullpath[STRINGSIZE];
  char dirpath[STRINGSIZE];
  int partition;
  int filetype;
  struct file_info* children[MAX_NUMBER_OF_FILES_IN_DIRECTORY]; //the culprit
  int number_of_children;
} file_info;

/* structs for passing data to other threads and callbacks */





////////////////////////////////////////////////////////////////////

/*  global variables  */
extern file_info* root_directory;
extern gboolean toggle_camera_lcd_screen_is_on;
extern GtkWidget* m_directory_tree;


extern GdkPixbuf* icon_blankdir;

extern GdkPixbuf* icon_avifile;
extern GdkPixbuf* icon_binfile;
extern GdkPixbuf* icon_jpgfile;
extern GdkPixbuf* icon_txtfile;
extern GdkPixbuf* icon_wavfile;
extern GdkPixbuf* icon_zbmfile;


extern gboolean flipper_capture; //does user wish to stop video capture?


////////////////////////////////////////////////////////////

/*  function prototypes */

/*  which are called by buttons  */

gboolean toggle_camera_lcd_screen( GtkWidget *widget,
				   GdkEvent *event,
				   gpointer data);
gboolean download_memory(GtkWidget *widget,
			 GdkEvent *event,
			 gpointer data);
gboolean powerdown_camcorder(GtkWidget *widget,
			     GdkEvent* event,
			     gpointer data);
gboolean open_camcorder( GtkWidget *widget,
			 GdkEvent *event,
			 gpointer data);
gboolean close_camcorder( GtkWidget *widget,
			  GdkEvent *event,
			  gpointer data);
gboolean unlock_camcorder( GtkWidget *widget,
			   GdkEvent *event,
			   gpointer data);

gboolean format_camcorder( GtkWidget *widget,
			   GdkEvent *event,
			   gpointer data);
gboolean send_monitor_command( GtkWidget *widget,
			       GdkEvent *event,
			       gpointer data);
gboolean capture_video( GtkWidget *widget,
			GdkEvent *event,
			gpointer data);
gboolean download_last_movie (GtkWidget *widget,
			      GdkEvent *event,
			      gpointer data);
gboolean download_all_movies (GtkWidget *widget,
			      GdkEvent *event,
			      gpointer data);
gboolean download_file(GtkWidget *widget,
		       GdkEvent *event,
		       gpointer data);
void download_file_start_thread(gpointer data);
gboolean upload_file( GtkWidget *widget,
		      GdkEvent *event,
		      gpointer data);
gboolean update_directory_listing (GtkWidget *widget,
				   GdkEvent *event,
				   gpointer data);

gboolean change_camera_settings(GtkWidget *widget, GdkEvent *event, gpointer data);
gboolean download_flash(GtkWidget *widget, GdkEvent *event, gpointer data);
gboolean enable_mass_storage(GtkWidget *widget, GdkEvent *event, gpointer data);
gboolean get_clock(GtkWidget *widget, GdkEvent *event, gpointer data);
gboolean set_clock(GtkWidget *widget, GdkEvent *event, gpointer data);
void create_and_display_button(void* func, const char* caption, GtkWidget* box, GtkWidget** button);
gboolean delete_file(GtkWidget* widget,
		     GdkEvent* event,
		     gpointer data);

 
/*  accessory functions to functions called from buttons  */
gboolean enable_buttons (GtkWidget* widget,
			 GdkEvent *event,
			 gpointer data);


/*  GUI-manipulating functions  */
void EnableControls(gboolean value);

/*  GUI-independent workhorse functions  */
gboolean ChangePartition(unsigned int partition); 
gboolean ChangeDirectory(const char* st);  

gboolean Close(void);
gboolean Monitor(const char* command);

gboolean DownloadFile(char* saveto, char* filename, int filesize);
void DownloadAllMovies(const char* foldername);
gboolean GetFileInfo(file_info* thisfileinfo, gboolean isfirstfile);
gboolean GetLastFileInfo(file_info* thisfileinfo);

gboolean GetAnyFileInfo(const char* filename, file_info *thisfileinfo);
gboolean PowerdownCamcorder(void);
gboolean CaptureVideo(const char* filename);

#endif
