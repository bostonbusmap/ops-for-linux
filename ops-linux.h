#ifndef OPS_LINUX
#define OPS_LINUX

////////////////////////////////////////////////////////
// big-endian and little-endian

#define bswap16(x) ((unsigned short)( (((x)&0xffu)<<8u) | (((x)>>8u)&0xffu) ))

#define bswap32(x) (\
  ((x)&0xff000000u) >> 24 \
  |                       \
  ((x)&0x00ff0000u) >>  8 \
  |                       \
  ((x)&0x0000ff00u) <<  8 \
  |                       \
  ((x)&0x000000ffu) << 24 \
)

// these are compiler-defined, so we put this section BEFORE the includes

#if defined(_WIN32) || defined(_WIN64)
#define __BYTE_ORDER 1
#define __BIG_ENDIAN 0
#define __LITTLE_ENDIAN 1
#else
#include <endian.h>
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
# define cpu_to_le32(x) bswap32(x)
# define le32_to_cpu(x) bswap32(x)
# define cpu_to_le16(x) bswap16(x)
# define le16_to_cpu(x) bswap16(x)
# define cpu_to_be32(x) (x)
# define be32_to_cpu(x) (x)
# define cpu_to_be16(x) (x)
# define be16_to_cpu(x) (x)
#else
# define cpu_to_le32(x) (x)
# define le32_to_cpu(x) (x)
# define cpu_to_le16(x) (x)
# define le16_to_cpu(x) (x)
# define cpu_to_be32(x) bswap32(x)
# define be32_to_cpu(x) bswap32(x)
# define cpu_to_be16(x) bswap16(x)
# define be16_to_cpu(x) bswap16(x)
#endif

/////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <usb.h>

//////////////////////////////////////////////////////////

/*  definitions  */
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

#define STRINGSIZE 128

#define TIMEOUT 4000

/////////////////////////////////////////////////////////////////

/*  new structs  */
typedef struct {
  GtkWidget* a;
  GtkWidget* b;
} double_widget; //for returning two widgets without a global variable

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

////////////////////////////////////////////////////////////////////

/*  global variables  */
extern struct usb_device* m_usb_device;
extern GtkWidget *main_window;
extern file_info* root_directory;
extern gboolean toggle_camera_lcd_screen_is_on;
extern GtkWidget *m_ctl_progress;
extern GtkWidget* m_directory_tree;
extern double m_progressbar_fraction;

extern usb_dev_handle *m_p_handle;



//unsigned short m_vendor_id;
//unsigned short m_product_id;
//char m_manufacturer[STRINGSIZE];
//char m_product[STRINGSIZE];

extern gboolean flipper_capture;
extern int stopwatch;   //maybe time downloads in the future and printout a bitrate


////////////////////////////////////////////////////////////

/*  function prototypes */
void Log(const char* stuff);  //to stderr at present moment

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
gboolean upload_file( GtkWidget *widget,
		      GdkEvent *event,
		      gpointer data);
gboolean update_directory_listing (GtkWidget *widget,
				   GdkEvent *event,
				   gpointer data);

gboolean change_camera_settings(GtkWidget *widget, GdkEvent *event, gpointer data);

 
/*  accessory functions to functions called from buttons  */
gboolean enable_buttons (GtkWidget* widget,
			 GdkEvent *event,
			 gpointer data);


/*  GUI-manipulating functions  */
gboolean MessageBox(const char* st);
gboolean MessageBoxChoice(const char* st, gpointer data);
gboolean MessageBoxConfirm(const char* st);
void EnableControls(gboolean value);


/*  GUI-independent workhorse functions  */
gboolean ChangePartition(unsigned int partition); 
gboolean ChangeDirectory(const char* st);  
gboolean ControlMessageWrite(unsigned int command, const char *const data, int size, int timeout);
gboolean ControlMessageRead(unsigned int command, char* data, int size, int timeout);
int Write(const char *const p_buffer, unsigned int length, int timeout);
int Read(char *p_buffer, unsigned int length, int timeout);

gboolean CheckCameraOpen(void);
gboolean Close(void);
gboolean Monitor(const char* command);
gboolean MessageBoxText (const char* st, gpointer data);
gboolean MessageBoxTextTwo (const char* st, gpointer data);
gboolean DownloadFile(char* saveto, char* filename);
gboolean GetFileInfo(file_info* thisfileinfo, gboolean isfirstfile);

gboolean delete_file(GtkWidget* widget,
               GdkEvent* event,
               gpointer data);

#endif
