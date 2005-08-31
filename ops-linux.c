#include "ops-linux.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

//////////////////////////////////////////
// various true (multi-file) globals
struct usb_device* m_usb_device;
GtkWidget *main_window = NULL;
file_info* root_directory;
gboolean toggle_camera_lcd_screen_is_on;
GtkWidget *m_ctl_progress = NULL;
GtkWidget* m_directory_tree = NULL;
double m_progressbar_fraction;

usb_dev_handle *m_p_handle;

gboolean flipper_capture;
int stopwatch;   //maybe time downloads in the future and printout a bitrate

//////////////////////////////////////////

static GtkWidget * button_open_camcorder, 
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
  *button_toggle_camera_lcd_screen,
  *button_download_memory,
  *button_powerdown_camcorder,
  *button_capture_video,
  *information_label;


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


static gboolean watch_progress_bar (gpointer data) {
  // quiet compiler
  data=data;
  
  gtk_progress_bar_set_fraction(m_ctl_progress, m_progressbar_fraction);
  
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
    p = data;
    snprintf(tempstring, STRINGSIZE - 1, "%s %d bytes", filename, p->filesize);
    gtk_label_set_text(GTK_LABEL(information_label), tempstring);
  }
}


static gboolean do_download = FALSE;
static gboolean do_format = FALSE;

static void process_args(int argc, char * argv[])
{
  int c;

  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
      {"download", 0, 0, 'd'},
      {"format", 0, 0, 'f'},
      {0, 0, 0, 0}
    };

    c = getopt_long (argc, argv, "df", long_options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 'd':
        do_download = TRUE;
        break;

      case 'f':
        do_format = TRUE;
        break;

      case '?':
        break;

      default:
        printf ("?? getopt returned character code 0%o ??\n", 
c);
    }
  }

  if (optind < argc) {
    printf ("non-option ARGV-elements: ");
    while (optind < argc) {
      printf ("%s ", argv[optind++]);
      printf ("\n");
    }
  }
}


void EnableControls(gboolean value) {

  if (main_window) {
    gtk_widget_set_sensitive(button_open_camcorder, value);
    gtk_widget_set_sensitive(button_unlock, value);
    gtk_widget_set_sensitive(button_close_camcorder, value);
    gtk_widget_set_sensitive(button_download_all_movies, value);
    gtk_widget_set_sensitive(button_download_last_movie, value);
    //  gtk_widget_set_sensitive(button_upload_movie, value);
    gtk_widget_set_sensitive(button_format_storage, value);
    gtk_widget_set_sensitive(button_delete_file, value);
    gtk_widget_set_sensitive(button_download_file, value);
    gtk_widget_set_sensitive(button_upload_file, value);
    gtk_widget_set_sensitive(button_send_monitor_command, value);
    gtk_widget_set_sensitive(button_update_directory_listing, value);
    gtk_widget_set_sensitive(m_directory_tree, value);
    gtk_widget_set_sensitive(button_toggle_camera_lcd_screen, value);
    gtk_widget_set_sensitive(button_capture_video, value);
  }
}


int main (int argc, char *argv[])
{

  /* GtkWidget is the storage type for widgets */
  
  GtkWidget * vbox1, *hbox2, *hbox3, *hbox1, *hbox_progressbar, *hbox_tree, *hbox_label;
  GtkWidget* window;
  GtkWidget* s_w;
  GtkTreeViewColumn *col;
  GtkCellRenderer *renderer;
  GtkObject* hadjustment, *vadjustment;
  //GError* error = NULL;
  //camcorder_files_size = 0;
  g_thread_init(NULL);
  gdk_threads_init();
  
  root_directory = NULL;
  /* This is called in all GTK applications. Arguments are parsed
   * from the command line and are returned to the application. */
  gtk_init (&argc, &argv);
  toggle_camera_lcd_screen_is_on = TRUE;
 
  // Handle the command line args
  process_args(argc,argv);
  
  if (do_download || do_format) {
    int ret=0;
    
    if (open_camcorder(NULL,NULL,NULL)) {
      if (unlock_camcorder(NULL,NULL,NULL)) {

        if (do_download && !download_all_movies(NULL,NULL,NULL)) ret=1;
        if (do_format && !format_camcorder(NULL,NULL,NULL)) ret=1;
      }
      else ret=1;

      close_camcorder(NULL,NULL,NULL);
    }
    else ret=1;
    
    exit(ret);
  }
  
  
  stopwatch = 0;
    /* create a new window */
  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  window = main_window;
  gtk_window_set_title (GTK_WINDOW (window), "Ops for linux");
  
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
  hbox1 = gtk_hbox_new (FALSE, 0);
  hbox_tree = gtk_hbox_new(FALSE,0);
  hbox_label = gtk_hbox_new(FALSE, 0);
  hbox_progressbar = gtk_hbox_new (FALSE, 0);
  
  

  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox2, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox3, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox_progressbar, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox_tree, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox_label, TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox1);
  

  hadjustment = gtk_adjustment_new(0,-1,1,.1,.5,1);
  vadjustment = gtk_adjustment_new(0,-1,1,.1,.5,1);

  
  m_directory_tree = gtk_tree_view_new();
  //  m_directory_model = NULL;
  gtk_widget_set_size_request (m_directory_tree,
			       300,
			       300);

  s_w = gtk_scrolled_window_new(hadjustment, vadjustment);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(s_w),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  //  gtk_container_set_height (GTK_TREE_VIEW (m_directory_tree), 300);

  //COLUMN 1
  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "filename");
  gtk_tree_view_append_column(GTK_TREE_VIEW(m_directory_tree), col);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(col, renderer, TRUE);
  gtk_tree_view_column_add_attribute(col, renderer, "text", COL_FILENAME);

  //COLUMN 2
  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "pointers");
  gtk_tree_view_append_column(GTK_TREE_VIEW(m_directory_tree), col);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(col, renderer, TRUE);
  //  gtk_tree_view_column_add_attribute(col, renderer, "text", COL_POINTER);
  /*if (!g_thread_create(watch_progress_bar, NULL, FALSE, &error)) {
    Log(error->message);
    //go without if error
    }*/
  
  
  information_label = gtk_label_new("");


  m_ctl_progress = gtk_progress_bar_new();

    /* Sets the border width of the window. */
    //gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    /* Creates a new button with the label "Hello World". */
  button_capture_video = gtk_button_new_with_label("Capture Video");
  button_open_camcorder = gtk_button_new_with_label ("Open Camcorder");
  button_unlock = gtk_button_new_with_label ("Unlock");
  button_close_camcorder = gtk_button_new_with_label ("Close Camcorder");
  button_download_all_movies = gtk_button_new_with_label ("Download All Movies");
  button_download_last_movie = gtk_button_new_with_label ("Download Last Movie");
  button_delete_file = gtk_button_new_with_label ("Delete File");
  button_format_storage = gtk_button_new_with_label ("Format Storage");
  button_send_monitor_command = gtk_button_new_with_label ("Send Monitor Command");
  button_update_directory_listing = gtk_button_new_with_label ("Update Directory Listing");
  button_download_file = gtk_button_new_with_label ("Download file");
  button_upload_file = gtk_button_new_with_label ("Upload file");
  button_toggle_camera_lcd_screen = gtk_button_new_with_label ("Toggle camera lcd screen");
  button_download_memory = gtk_button_new_with_label("Download memory");
  button_powerdown_camcorder = gtk_button_new_with_label("Powerdown camcorder");
  gtk_signal_connect (GTK_OBJECT (button_open_camcorder), "clicked", 
		      GTK_SIGNAL_FUNC(open_camcorder), NULL);
  gtk_signal_connect (GTK_OBJECT (button_unlock), "clicked",
		      GTK_SIGNAL_FUNC(unlock_camcorder), NULL);
  gtk_signal_connect (GTK_OBJECT (button_close_camcorder), "clicked",
		      GTK_SIGNAL_FUNC(close_camcorder), NULL);
  gtk_signal_connect (GTK_OBJECT (button_download_all_movies), "clicked",
  		      GTK_SIGNAL_FUNC (download_all_movies), NULL);
  gtk_signal_connect (GTK_OBJECT (button_download_last_movie), "clicked",
		      GTK_SIGNAL_FUNC(download_last_movie), NULL);
  // g_signal_connect (G_OBJECT (button_upload_movie), "clicked",
  //	    G_CALLBACK (hello), NULL);
  gtk_signal_connect (GTK_OBJECT (button_format_storage), "clicked",
		      GTK_SIGNAL_FUNC(format_camcorder), NULL);
  
  gtk_signal_connect_object (GTK_OBJECT (button_close_camcorder), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (window));
  gtk_signal_connect (GTK_OBJECT (button_send_monitor_command), "clicked",
		      GTK_SIGNAL_FUNC(send_monitor_command), NULL);
  gtk_signal_connect (GTK_OBJECT (button_update_directory_listing), "clicked",
		      GTK_SIGNAL_FUNC(update_directory_listing), NULL);
  gtk_signal_connect (GTK_OBJECT (button_download_file), "clicked",
		      GTK_SIGNAL_FUNC(download_file), NULL);
  gtk_signal_connect (GTK_OBJECT (button_upload_file), "clicked",
		      GTK_SIGNAL_FUNC(upload_file), NULL);
  gtk_signal_connect (GTK_OBJECT (button_delete_file), "clicked",
		      GTK_SIGNAL_FUNC(delete_file), NULL);
  gtk_signal_connect (GTK_OBJECT (m_directory_tree), "row-activated",
		      GTK_SIGNAL_FUNC(reset_label), NULL);
  gtk_signal_connect (GTK_OBJECT (button_toggle_camera_lcd_screen), "clicked",
		      GTK_SIGNAL_FUNC(toggle_camera_lcd_screen), NULL);
  gtk_signal_connect (GTK_OBJECT (button_download_memory), "clicked",
		      GTK_SIGNAL_FUNC(download_memory), NULL);
  gtk_signal_connect (GTK_OBJECT (button_powerdown_camcorder), "clicked",
		      GTK_SIGNAL_FUNC(powerdown_camcorder), NULL);
  gtk_signal_connect (GTK_OBJECT (button_capture_video), "clicked",
		      GTK_SIGNAL_FUNC(capture_video), NULL);

  
  gtk_box_pack_start (GTK_BOX (hbox1), button_open_camcorder, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox1), button_unlock, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox1), button_close_camcorder, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox1), button_powerdown_camcorder, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), button_download_all_movies, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), button_download_last_movie, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), button_delete_file, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), button_capture_video, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox3), button_format_storage, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_progressbar), m_ctl_progress, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox3), button_send_monitor_command, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox3), button_download_file, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_tree), s_w, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox3), button_upload_file, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_label), information_label, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_label), button_toggle_camera_lcd_screen, TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (s_w), m_directory_tree);

  //  gtk_box_pack_start (GTK_BOX (s_w), 
  gtk_box_pack_start (GTK_BOX (hbox_tree), button_update_directory_listing, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_tree), button_download_memory, TRUE, TRUE, 0);
  /*  gtk_widget_show (button_open_camcorder);
  gtk_widget_show (button_unlock);
  gtk_widget_show (button_close_camcorder);
  gtk_widget_show (button_download_all_movies);
  gtk_widget_show (button_download_last_movie);
  gtk_widget_show (button_upload_movie);
  gtk_widget_show (button_format_storage);
  gtk_widget_show (m_ctl_progress);
  gtk_widget_show (m_directory_tree);
  gtk_widget_show (button_send_monitor_command);
  gtk_widget_show (button_update_directory_listing);

  gtk_widget_show (hbox1);
  gtk_widget_show (hbox2);
  gtk_widget_show (hbox3);
  gtk_widget_show (hbox_progressbar);
  gtk_widget_show (hbox_tree);
  gtk_widget_show (vbox1);*/
  gtk_widget_show_all (window);
  //  gdk_threads_enter();
  m_progressbar_fraction = 0; // 0 % complete :)
  gtk_timeout_add(1000,GTK_SIGNAL_FUNC(watch_progress_bar), NULL);
  //if (!g_thread_create(watch_progress_bar, NULL, FALSE, &error)) {
  //Log(error->message);
    //go without if error
  //}
 

  gtk_main ();
  //  gdk_threads_leave();
  return 0;
}


