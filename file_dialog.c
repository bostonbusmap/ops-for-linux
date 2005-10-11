#include "ops-linux.h"

#ifndef USE_GTK_FILE_CHOOSER
#define GTK_FILE_CHOOSER_ACTION_SAVE 1
#define GTK_FILE_CHOOSER_ACTION_OPEN 2
#endif



void file_selection_ok (GtkWidget *widget, GtkFileSelection* info)
{
  const char* name;
  //  trace("");

  gtk_main_quit();
}



char* store_filename(GtkWidget* file_selection_box) {
  
  const gchar* name = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_box));
  char* ret_val = malloc(strlen(name) + 1);
  strcpy(ret_val, name);
  return ret_val;
}

char* get_filename_from_dialog(const char* filenamechoice, int action) {
  gchar* save_filename;
  char* filename_return;
  char* ret_val = NULL;
#ifdef USE_GTK_FILE_CHOOSER
  GtkWidget* file_selection_dialog =
    gtk_file_chooser_dialog_new("Please select a place to download to",
				GTK_WINDOW(main_window), //meaningless?
				action,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);
  if (filenamechoice != NULL)
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(file_selection_dialog), filenamechoice);
  if (gtk_dialog_run(GTK_DIALOG(file_selection_dialog)) == GTK_RESPONSE_ACCEPT) {
    save_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_selection_dialog));
    filename_return = (char*)malloc(strlen(save_filename) + 1);
    if (filename_return == NULL) { //no memory left, maybe
      return NULL; //just give up
    }
    strcpy(filename_return, save_filename);
    gtk_widget_destroy(file_selection_dialog);
    return filename_return;
  } else { //user cancelled
    gtk_widget_destroy(file_selection_dialog);
    
    return NULL;
  }
#else
  GtkFileSelection* file_selection_box =  gtk_file_selection_new("Choose a file");
  /*  g_signal_connect (GTK_FILE_SELECTION(file_selection_box)->ok_button,
		    "clicked",
		    G_CALLBACK(store_filename),
		    file_selection_box);*/

  
  //  gtk_window_set_modal(GTK_WINDOW(file_selection_box), TRUE);

  gtk_signal_connect (GTK_OBJECT (file_selection_box), "delete_event",
		      GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
  gtk_signal_connect (GTK_OBJECT (file_selection_box->ok_button), "clicked",
		      GTK_SIGNAL_FUNC(file_selection_ok),
		      file_selection_box);
  gtk_signal_connect (GTK_OBJECT (file_selection_box->cancel_button), "clicked",
		      GTK_SIGNAL_FUNC(gtk_main_quit), NULL);



  gtk_window_set_modal (GTK_WINDOW (file_selection_box), TRUE);

  gtk_widget_show(GTK_WIDGET(file_selection_box));
  
  gtk_grab_add (GTK_WIDGET (file_selection_box));
  gtk_main ();
  ret_val = store_filename(file_selection_box);
  gtk_widget_destroy(file_selection_box);
  return ret_val;
#endif
}





char* get_download_filename(const char* filenamechoice) {
  return get_filename_from_dialog(filenamechoice, GTK_FILE_CHOOSER_ACTION_SAVE);

}

char* get_upload_filename() {
  return get_filename_from_dialog(NULL, GTK_FILE_CHOOSER_ACTION_OPEN);

}
