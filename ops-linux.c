#include "ops-linux.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

//////////////////////////////////////////
// various true (multi-file) globals
file_info* root_directory;
gboolean toggle_camera_lcd_screen_is_on;

GtkWidget* m_directory_tree;

gboolean flipper_capture;

GdkPixbuf* icon_blankdir;
GdkPixbuf* icon_avifile;
GdkPixbuf* icon_binfile;
GdkPixbuf* icon_jpgfile;
GdkPixbuf* icon_txtfile;
GdkPixbuf* icon_wavfile;
GdkPixbuf* icon_zbmfile;

//////////////////////////////////////////

typedef struct {
  GtkWidget *button_open_camcorder;
  GtkWidget *button_unlock_camcorder;
  GtkWidget *button_close_camcorder;
  GtkWidget *button_download_all_movies;
  GtkWidget *button_change_camera_settings;
  GtkWidget *button_download_last_movie;
  GtkWidget *button_delete_file;
  GtkWidget *button_format_camcorder;
  GtkWidget *button_send_monitor_command;
  GtkWidget *button_update_directory_listing;
  GtkWidget *button_download_file;
  GtkWidget *button_upload_file;
  GtkWidget *button_toggle_camera_lcd_screen;
  GtkWidget *button_download_memory;
  GtkWidget *button_powerdown_camcorder;
  GtkWidget *button_capture_video;
  GtkWidget *button_download_flash;
  GtkWidget *button_enable_mass_storage;
  GtkWidget *button_get_clock;
  GtkWidget *button_set_clock;
} button_set;
#define NUM_OF_BUTTONS ( sizeof(button_set) / sizeof(GtkWidget*) )

union button_union {
  button_set b_s;
  GtkWidget* buttons[NUM_OF_BUTTONS];
} b_u;

static GtkWidget *information_label;


static gboolean delete_event (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  // quiet compiler
  widget=widget;
  event=event;
  data=data;

  return FALSE;
}


/* Another callback */
static void destroy (GtkWidget * widget, gpointer data)
{
  // quiet compiler
  widget=widget;
  data=data;

  gtk_main_quit ();
  
}

gboolean enable_buttons (GtkWidget* widget,
			 GdkEvent *event,
			 gpointer data) {
  // quiet compiler
  widget=widget;
  event=event;
  data=data;
  
  EnableControls(TRUE);
  return TRUE;

}




static void reset_label(GtkTreeView* treeview,
		 GtkTreePath *arg1,
		 GtkTreeViewColumn *arg2,
		 gpointer data_null) {
  GtkTreeSelection* selection;
  GtkTreeIter iter;
  GtkTreeModel* model;
  gpointer data;
  gchar* filename;
  char strfiletype[STRINGSIZE];
  // quiet compiler
  treeview=treeview;
  arg1=arg1;
  arg2=arg2;
  data_null=data_null;

  

  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_directory_tree));
  if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    char tempstring[STRINGSIZE];
    //    gpointer data = NULL;
    file_info* p = NULL;
   
    gtk_tree_model_get (model, &iter, COL_FILENAME, &filename,
			COL_POINTER, &data, -1);
    p = (file_info*)data;
    switch (p->filetype) {
    case FIROOT: //shouldn't happen
      strcpy(strfiletype, "ROOT");
      break;
    case FIDIR:
      strcpy(strfiletype, "DIRECTORY");
      break;
    case FIFILE:
      strcpy(strfiletype, "FILE");
      break;
    case FIPART:
      strcpy(strfiletype, "PARTITION");
      break;
    default:
      strcpy(strfiletype, "UNKNOWN");
      break;
    };
    
    snprintf(tempstring, STRINGSIZE - 1, "%s %d bytes   %s", filename, p->filesize, strfiletype);
    gtk_label_set_text(GTK_LABEL(information_label), tempstring);
  }
}


static gboolean do_download = FALSE;
static gboolean do_format = FALSE;
static gboolean do_help = FALSE;

static void process_args(int argc, char * argv[])
{
  int c;

  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
      {"download", 0, 0, 'd'},
      {"format", 0, 0, 'f'},
      {"help", 0, 0, 'h'},
      {0, 0, 0, 0}
    };

    c = getopt_long (argc, argv, "dfh", long_options, &option_index);
    if (c == -1) break;
    
    switch (c) {
    case 'd':
      do_download = TRUE;
      break;

    case 'f':
      do_format = TRUE;
      break;
      
    case 'h':
      do_help = TRUE;
      break;
      
    case '?':
      break;
      
    default:
      Log(ERROR, "getopt returned character code 0x%2x", c);
    }
  }

  if (optind < argc) {
    Log(DEBUGGING, "non-option ARGV-elements: ");
    while (optind < argc) {
      Log(DEBUGGING, "%s ", argv[optind++]);
    }
  }
}


void EnableControls(gboolean value) {
  if (main_window) {
    unsigned int count;
    for (count = 0; count < NUM_OF_BUTTONS; ++count) {
      //      fprintf(stderr, "button: %08x\n", b_u.buttons[count]);
      gtk_widget_set_sensitive(b_u.buttons[count], value);

    }
  }
}
void EnableOpenButton(void) {
  if (main_window) {
    gtk_widget_set_sensitive(b_u.b_s.button_open_camcorder, TRUE);

  }
}

extern const guint8 avifile[], binfile[], blankdir[], jpgfile[];
extern const guint8 txtfile[], wavfile[], zbmfile[];

static void load_file_icons(void) {
  icon_blankdir = gdk_pixbuf_new_from_inline(-1, blankdir, FALSE, NULL);

  icon_avifile = gdk_pixbuf_new_from_inline(-1, avifile, FALSE, NULL);
  icon_binfile = gdk_pixbuf_new_from_inline(-1, binfile, FALSE, NULL);
  icon_jpgfile = gdk_pixbuf_new_from_inline(-1, jpgfile, FALSE, NULL);
  icon_txtfile = gdk_pixbuf_new_from_inline(-1, txtfile, FALSE, NULL);
  icon_wavfile = gdk_pixbuf_new_from_inline(-1, wavfile, FALSE, NULL);
  icon_zbmfile = gdk_pixbuf_new_from_inline(-1, zbmfile, FALSE, NULL);
}

void create_and_display_button(void* func, const char* caption, GtkWidget* box, GtkWidget** button) {
  *button = gtk_button_new_with_label(caption);
  gtk_signal_connect (GTK_OBJECT (*button), "clicked",
		      GTK_SIGNAL_FUNC(func), NULL);

  gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET(*button), TRUE, TRUE, 0);
}

#define create_and_display_button_short(func, caption, box) \
  create_and_display_button(func, caption, box, &( b_u.b_s.button_##func ));



int main (int argc, char *argv[]) {

  /* GtkWidget is the storage type for widgets */
  
  GtkWidget * vbox1, *hbox2, *hbox3, *hbox4, *hbox5, *hbox1, *hbox_progressbar, *hbox_tree, *hbox_label;
  GtkWidget* window;
  GtkWidget* s_w;
  GtkTreeViewColumn *col;
  GtkCellRenderer *renderer;
  GtkObject* hadjustment, *vadjustment;
  //GError* error = NULL;
  //camcorder_files_size = 0;
  Log(DEBUGGING, "Starting threading...");
  g_thread_init(NULL);
  gdk_threads_init();
  Log(DEBUGGING, "Threading started");
  root_directory = NULL;
  set_bitrate(0);
  /* This is called in all GTK applications. Arguments are parsed
   * from the command line and are returned to the application. */
  Log(DEBUGGING, "Starting gtk...");
  gtk_init (&argc, &argv);
  toggle_camera_lcd_screen_is_on = TRUE;
 
  // Handle the command line args
  process_args(argc,argv);
  
  if (do_help) {
    Log(NOTICE, "flags:");
    Log(NOTICE, "-d -- download all movies from the camcorder");
    Log(NOTICE, "-f -- clear camera's movie partition (erase all movies)");
    Log(NOTICE, "-h -- print this help info");
    Log(NOTICE, "Flags may be combined to get a combined effect");
    exit(0);
  } else if (do_download || do_format) {
    int ret=0;
    
    if (open_camcorder(NULL,NULL,NULL)) {
      if (unlock_camcorder(NULL,NULL,NULL)) {

        if (do_download) {
	  DownloadAllMovies(".");
	  ret=1;
	}
        if (do_format && !format_camcorder(NULL,NULL,NULL)) ret=1;
      }
      else ret=1;

      close_camcorder(NULL,NULL,NULL);
    }
    else ret=1;
    
    exit(ret);
  }
  
    /* create a new window */
  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  window = main_window;
  gtk_window_set_title (GTK_WINDOW (window), "Ops for linux");
  Log(DEBUGGING, "main window created");
    /* When the window is given the "delete_event" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
  gtk_signal_connect (GTK_OBJECT (window), "delete_event",GTK_SIGNAL_FUNC(delete_event), NULL);
    /* Here we connect the "destroy" event to a signal handler.  
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete_event" callback. */
  gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (destroy), NULL);
  vbox1 = gtk_vbox_new (FALSE, 0);
  hbox2 = gtk_hbox_new (FALSE, 0);
  hbox3 = gtk_hbox_new (FALSE, 0);
  hbox4 = gtk_hbox_new (FALSE, 0);
  hbox5 = gtk_hbox_new (FALSE, 0);
  hbox1 = gtk_hbox_new (FALSE, 0);
  hbox_tree = gtk_hbox_new(FALSE,0);
  hbox_label = gtk_hbox_new(FALSE, 0);
  hbox_progressbar = gtk_hbox_new (FALSE, 0);
  
  

  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox2, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox3, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox4, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox5, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox_tree, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox_label, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox_progressbar, TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox1);
  

  hadjustment = gtk_adjustment_new(0,-1,1,.1,.5,1);
  vadjustment = gtk_adjustment_new(0,-1,1,.1,.5,1);

  
  m_directory_tree = gtk_tree_view_new();
  //  m_directory_model = NULL;
  gtk_widget_set_size_request (m_directory_tree,
			       300,
			       300);

  s_w = gtk_scrolled_window_new(GTK_ADJUSTMENT(hadjustment), 
				GTK_ADJUSTMENT(vadjustment));
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(s_w),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  //  gtk_container_set_height (GTK_TREE_VIEW (m_directory_tree), 300);

  //ICON COLUMN
  col = gtk_tree_view_column_new();
  gtk_tree_view_append_column(GTK_TREE_VIEW(m_directory_tree), col);
  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_column_pack_start(col, renderer, FALSE);
  gtk_tree_view_column_add_attribute(col, renderer, "pixbuf", COL_ICON);



  //COLUMN 1
  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "filename");
  gtk_tree_view_append_column(GTK_TREE_VIEW(m_directory_tree), col);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(col, renderer, TRUE);
  gtk_tree_view_column_add_attribute(col, renderer, "text", COL_FILENAME);

  //COLUMN 2
  col = gtk_tree_view_column_new();
  gtk_tree_view_append_column(GTK_TREE_VIEW(m_directory_tree), col);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(col, renderer, TRUE);

  
  //  gtk_tree_view_column_add_attribute(col, renderer, "text", COL_POINTER);
  /*if (!g_thread_create(watch_progress_bar, NULL, FALSE, &error)) {
    Log(error->message);
    //go without if error
    }*/
  
  load_file_icons();

  information_label = gtk_label_new("");
  bitrate_label = gtk_label_new("");

  m_ctl_progress = gtk_progress_bar_new();

  create_and_display_button_short(open_camcorder, "Open Camcorder", hbox1);
  create_and_display_button_short(unlock_camcorder, "Unlock", hbox1);
  create_and_display_button_short(close_camcorder, "Close Camcorder", hbox1);
  create_and_display_button_short(download_all_movies, "Download All Movies", hbox2);
  create_and_display_button_short(download_last_movie, "Download Last Movie", hbox2);
  create_and_display_button_short(update_directory_listing, "Update Directory Listing", hbox5);
  create_and_display_button_short(download_file, "Download file", hbox2);
  create_and_display_button_short(upload_file, "Upload file", hbox2);
  create_and_display_button_short(delete_file, "Delete File", hbox2);
  create_and_display_button_short(format_camcorder, "Format Storage", hbox2);
  create_and_display_button_short(send_monitor_command, "Send Monitor Command", hbox3);
  create_and_display_button_short(change_camera_settings, "Camera settings", hbox4);
  create_and_display_button_short(toggle_camera_lcd_screen, "Toggle camera lcd screen", hbox3);
  create_and_display_button_short(download_memory, "Download memory", hbox3);
  create_and_display_button_short(powerdown_camcorder, "Powerdown camcorder", hbox3);
  create_and_display_button_short(download_flash,"Download flash", hbox3);
  create_and_display_button_short(enable_mass_storage, "Enable Mass Storage", hbox4);
  create_and_display_button_short(get_clock, "Get Clock value", hbox4);
  create_and_display_button_short(set_clock, "Set Clock value", hbox4);
  create_and_display_button_short(capture_video, "Capture Video", hbox3);
  //  debug_buttons();
#ifndef DEBUG  
  EnableControls(FALSE);
  EnableOpenButton();
#endif
  gtk_signal_connect (GTK_OBJECT (m_directory_tree), "cursor-changed", /* "row-activated",*/
		      GTK_SIGNAL_FUNC(reset_label), NULL);

  gtk_box_pack_start (GTK_BOX (hbox_tree), s_w, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_label), information_label, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_label), bitrate_label, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_progressbar), m_ctl_progress, TRUE, TRUE, 0);
  
  gtk_container_add (GTK_CONTAINER (s_w), GTK_WIDGET(m_directory_tree));

  gtk_widget_show_all (window);

  gtk_timeout_add(REFRESH_DATA_MS,(GtkFunction)watch_progress_bar, NULL);
  
#ifndef DEBUG
  //open camcorder by default
  open_camcorder(NULL,NULL,NULL);
  //unlock by default (as of sep 28, no camcorder is permanently unlocked)
  unlock_camcorder(NULL,NULL,NULL);
#endif
  gdk_threads_enter();
  gtk_main ();
  gdk_threads_leave();
  return 0;
}
