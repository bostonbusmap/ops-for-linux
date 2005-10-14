#ifndef WIDGETS_H
#define WIDGETS_H
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "../log.h"

#define REFRESH_DATA_MS 250

#if (GTK_CHECK_VERSION(2,4,0))
#define USE_GTK_FILE_CHOOSER
#endif


extern GtkWidget* main_window;
extern GtkWidget *m_ctl_progress;
extern double m_progressbar_fraction;
extern int m_previous_bytes;
extern int m_current_bytes;
extern GtkWidget  *bitrate_label;



/* structs for passing data to other threads and callbacks */
typedef struct {
  void* a;
  void* b;

} twosome;
typedef struct {
  void* a;
  void* b;
  void* c;
} threesome;
typedef struct {
  void* a;
  void* b;
  void* c;
  void* d;
} foursome;
/*  new structs  */
typedef struct {
  GtkWidget* a;
  GtkWidget* b;
} double_widget; //for returning two widgets without a global variable


int text_option_box(int number_of_options, const char* st, ...);
gboolean MessageBoxTextTwo (const char* st, gpointer data);
gboolean MessageBox(const char *st);
char* MessageBoxText(const char* st);  //NOTE: returns malloced info
gboolean MessageBoxConfirm (const char* st);
gboolean set_progress_bar(double value);
gboolean set_bitrate(double kbyterate);
gboolean watch_progress_bar (gpointer data);



#endif
