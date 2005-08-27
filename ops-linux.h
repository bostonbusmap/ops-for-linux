#ifndef OPS_LINUX
#define OPS_LINUX
#include <gtk/gtk.h>
#include <usb.h>
#include <stdio.h>
#include <string.h>
#define VENDOROLD 0x04c5 //Fujitsu (testmarket revision Saturn)
#define VENDOR 0x167b //Pure Digital (B1,B2,B3 revision Saturn)

#define READ_ENDPOINT			0x81 
#define WRITE_ENDPOINT			0x01

#define DEFAULT_CONFIGURATION	1
#define DEFAULT_INTERFACE	0
#define DEFAULT_ALT_INTERFACE	0

#define COL_FILENAME 0
#define COL_POINTER 1

#define SHORT_TIMEOUT			500
#define TIMEOUT					4000
#define LONG_TIMEOUT			14000
#define SUPER_TIMEOUT			50000

#define LIBUSB_PATH_MAX 4096 

//#define BUFSIZE					4096
#define BUFSIZE 64 //it's a linux thing... :(
#define STRINGSIZE 128

#define TIMEOUT 4000

struct usb_device*	m_usb_device;
usb_dev_handle *m_p_handle;

unsigned short m_vendor_id;
unsigned short m_product_id;

char m_manufacturer[STRINGSIZE];
char m_product[STRINGSIZE];

GtkWidget * button_open_camcorder, 
  *button_unlock, 
  *button_close_camcorder, 
  *button_download_all_movies, 
  *button_download_last_movie, 
  *button_delete_file, 
  *button_format_storage,
  *button_send_monitor_command,
  *button_update_directory_listing,
  *button_download_file,
  *button_upload_file,
  *information_label,
  *main_window,
  *button_toggle_camera_lcd_screen,
  *button_download_memory,
  *button_powerdown_camcorder,
  *button_capture_video;

gboolean toggle_camera_lcd_screen_is_on;

extern GtkWidget *m_ctl_progress;

//GtkTreeView* m_directory_tree;
GtkWidget* m_directory_tree;
double m_progressbar_fraction;
//GtkTreeModel* m_directory_model;

typedef struct {
  char name[STRINGSIZE];
  gpointer function_pointer;

} callback_collection;

typedef struct {
  GtkWidget* a;
  GtkWidget* b;
} double_widget;

#define FIFILE	0
#define FIDIR	1
#define FIPART	2
#define FIROOT	3

#define MAX_NUMBER_OF_FILES_IN_DIRECTORY 1000

struct  file_info{
  char filename[STRINGSIZE];
  int filesize;
  char fullpath[STRINGSIZE];
  char dirpath[STRINGSIZE];
  int partition;
  int filetype;
  struct file_info* children[MAX_NUMBER_OF_FILES_IN_DIRECTORY];
  int number_of_children;
};
typedef struct file_info file_info;

gboolean flipper_capture;
int stopwatch;
file_info* root_directory;
typedef struct {
  char text[STRINGSIZE];
} constant_string;

void Log(const char* stuff);
gboolean Init();
gboolean Open();
gboolean Close();
gboolean Unlock();
gboolean Format();
gboolean CheckCameraOpen();
//gboolean DownloadMovie();
gboolean start_download_all_movies(GtkWidget *widget,
				   GdkEvent *event,
				   gpointer data);
gboolean enable_buttons (GtkWidget* widget,
			 GdkEvent *event,
			 gpointer data);

gboolean toggle_camera_lcd_screen( GtkWidget *widget,
				   GdkEvent *event,
				   gpointer data);
gboolean download_memory(GtkWidget *widget,
			 GdkEvent *event,
			 gpointer data);
gboolean powerdown_camcorder(GtkWidget *widget,
			     GdkEvent* event,
			     gpointer data);
gboolean ControlMessageWrite(unsigned int command, int *data, int size, int timeout);
gboolean ControlMessageRead(unsigned int command, int* data, int size, int timeout);
int Write(unsigned char *p_buffer, unsigned int length, int timeout);
int Read(unsigned char *p_buffer, unsigned int length, int timeout);
constant_string GetMovieName(char* directory);
gboolean DownloadMovie(int videonumber, FILE* file);
gboolean MonitorCommand(char* command);
int GetMovieCount();
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
// update_directory_listing.c
gboolean GetFileInfo(file_info* thisfileinfo, gboolean isfirstfile);

// download_file.c
gboolean DownloadFile(char* saveto, char* filename);

 

gboolean format_camcorder_confirmed(GtkWidget *widget);
gboolean send_monitor_command_confirmed(GtkWidget *widget);
gboolean update_directory_listing (GtkWidget *widget,
				   GdkEvent *event,
				   gpointer data);
GtkTreeModel* create_model();
void RecursiveListing(char* parentpath, file_info* parent, GtkTreeIter* parent_place, int partition, int level, GtkTreeStore* treestore);
gboolean download_last_movie (GtkWidget *widget,
			      GdkEvent *event,
			      gpointer data);
gboolean download_all_movies (GtkWidget *widget,
			      GdkEvent *event,
			      gpointer data);
gboolean download_file(GtkWidget *widget,
		       GdkEvent *event,
		       gpointer data);
gboolean upload_file( GtkWidget *widget,
		      GdkEvent *event,
		      gpointer data);
gboolean delete_file(GtkWidget* widget,
		     GdkEvent* event,
		     gpointer data);
gboolean delete_file_confirmed( gpointer data);


gboolean download_file_store_filename(GtkWidget *widget);
gboolean download_last_movie_start(gpointer data);
gboolean download_file_start(gpointer data);
gboolean upload_file_start(gpointer data);
gboolean upload_file_confirmed(GtkWidget* widget);
gboolean ChangePartition(unsigned int partition);
gboolean ChangeDirectory(const char* st);
gboolean store_filename(GtkWidget* f, GdkEvent *event, gpointer data);
void destroy (GtkWidget * widget, gpointer data);
gboolean delete_event (GtkWidget * widget, GdkEvent * event, gpointer data);
gboolean MessageBox(const char* st);
gboolean MessageBoxChoice(const char* st, gpointer data);
void EnableControls(gboolean value);
int GetLength(FILE* file);
#endif
