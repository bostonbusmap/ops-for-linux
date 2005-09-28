#include "ops-linux.h"



char* get_filename_from_dialog(const char* filenamechoice, int action) {
  gchar* save_filename;
  char* filename_return;
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

}
char* get_download_filename(const char* filenamechoice) {
  return get_filename_from_dialog(filenamechoice, GTK_FILE_CHOOSER_ACTION_SAVE);

}

char* get_upload_filename() {
  return get_filename_from_dialog(NULL, GTK_FILE_CHOOSER_ACTION_OPEN);

}
