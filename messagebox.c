#include "ops-linux.h"

static double_widget global_dw;

gboolean MessageBoxTextTwo (const char* st, gpointer data) {
  GtkWidget *window, 
    *ok_button,
    *cancel_button,
    *label,
    *hbox, 
    *textentry_a,
    *textentry_b,
    *vbox, *hbox_textentry;
  //  gpointer data = cc->function_pointer;
  //  char* textbox = malloc(STRINGSIZE); //up to user to clean this up
  window = gtk_dialog_new ();
  
  ok_button = gtk_button_new_with_label ("OK");
  cancel_button = gtk_button_new_with_label("Cancel");
  global_dw.a = gtk_entry_new_with_max_length(255); //just to be safe
  textentry_a = global_dw.a;
  global_dw.b = gtk_entry_new_with_max_length(255);
  textentry_b = global_dw.b;
  //  textbox = gtk_entry_get_text(textentry);
  //printf("textentry: %08x\n",textentry);
  gtk_signal_connect_object (GTK_OBJECT (cancel_button),
			     "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy), 
			     GTK_OBJECT(window));
  
  
  gtk_signal_connect_object (GTK_OBJECT (ok_button),
			     "clicked",
			     GTK_SIGNAL_FUNC (data),
			     &global_dw);
  
  gtk_signal_connect_object (GTK_OBJECT (ok_button),
			     "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     GTK_OBJECT(window));

  hbox = gtk_hbox_new(FALSE,0);
  hbox_textentry = gtk_hbox_new(FALSE,0);
  vbox = gtk_vbox_new(FALSE,0);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(window)->action_area),
		      hbox, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_textentry),
		      textentry_a, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_textentry),
		      textentry_b, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox),
		      hbox_textentry, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox),
		     ok_button, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox),
		     cancel_button, TRUE, TRUE, 0);
  
  /*gtk_widget_show(ok_button);
  gtk_widget_show(cancel_button);
  gtk_widget_show(hbox);
  gtk_widget_show(hbox_textentry);
  gtk_widget_show(vbox);
  gtk_widget_show(textentry);*/

  label = gtk_label_new(st);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), label, TRUE, TRUE, 0);
  gtk_widget_show_all(window);
  //for now ignore the button part, use window manager button on top

  
  return TRUE;
  
}


gboolean MessageBox (const char *st)
{
  GtkWidget *window, *ok_button, *label;
  window = gtk_dialog_new ();

  ok_button = gtk_button_new_with_label ("OK");
  gtk_signal_connect_object (GTK_OBJECT (ok_button),
			     "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy), 
			     GTK_OBJECT(window));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->action_area),
		     ok_button, TRUE, TRUE, 0);
  gtk_widget_show(ok_button);
  //for now ignore the button part, use window manager button on top
  label = gtk_label_new (st);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), label, TRUE, TRUE, 0);
  
  gtk_widget_show (label);
  gtk_widget_show(window);
  return TRUE;

}


gboolean MessageBoxText (const char* st, gpointer data) {
  GtkWidget *window, 
    *ok_button,
    *cancel_button,
    *label,
    *hbox, 
    *textentry,
    *vbox, *hbox_textentry;
  //  gpointer data = cc->function_pointer;
  //char* textbox = malloc(STRINGSIZE); //up to user to clean this up
  const char* textbox;
  window = gtk_dialog_new ();
  
  ok_button = gtk_button_new_with_label ("OK");
  cancel_button = gtk_button_new_with_label("Cancel");
  textentry = gtk_entry_new_with_max_length(255); //just to be safe
  textbox = gtk_entry_get_text(textentry);
  //printf("textentry: %08x\n",textentry);
  gtk_signal_connect_object (GTK_OBJECT (cancel_button),
			     "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy), 
			     GTK_OBJECT(window));
  
  
  gtk_signal_connect_object (GTK_OBJECT (ok_button),
			     "clicked",
			     GTK_SIGNAL_FUNC (data),
			     textentry);
  
  gtk_signal_connect_object (GTK_OBJECT (ok_button),
			     "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     GTK_OBJECT(window));

  hbox = gtk_hbox_new(FALSE,0);
  hbox_textentry = gtk_hbox_new(FALSE,0);
  vbox = gtk_vbox_new(FALSE,0);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(window)->action_area),
		      hbox, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_textentry),
		      textentry, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox),
		      hbox_textentry, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox),
		     ok_button, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox),
		     cancel_button, TRUE, TRUE, 0);
  
  /*gtk_widget_show(ok_button);
  gtk_widget_show(cancel_button);
  gtk_widget_show(hbox);
  gtk_widget_show(hbox_textentry);
  gtk_widget_show(vbox);
  gtk_widget_show(textentry);*/

  label = gtk_label_new(st);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), label, TRUE, TRUE, 0);
  gtk_widget_show_all(window);
  //for now ignore the button part, use window manager button on top

  
  return TRUE;
  
}


gboolean MessageBoxChoice (const char* st, gpointer data) {
  GtkWidget *window, *ok_button, *cancel_button, *label, *hbox;
  //  gpointer data = cc->function_pointer;
  
  window = gtk_dialog_new ();
  
  ok_button = gtk_button_new_with_label ("OK");
  cancel_button = gtk_button_new_with_label("Cancel");
  gtk_signal_connect_object (GTK_OBJECT (cancel_button),
			     "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy), 
			     GTK_OBJECT(window));
  
  
  gtk_signal_connect_object (GTK_OBJECT (ok_button),
			     "clicked",
			     GTK_SIGNAL_FUNC (data),
			     NULL);
  
  gtk_signal_connect_object (GTK_OBJECT (ok_button),
			     "clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     GTK_OBJECT(window));

  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(window)->action_area),
		      hbox, TRUE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(hbox),
		     ok_button, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox),
		     cancel_button, TRUE, TRUE, 0);
  
  gtk_widget_show(ok_button);
  gtk_widget_show(cancel_button);
  gtk_widget_show(hbox);
  //for now ignore the button part, use window manager button on top
  label = gtk_label_new (st);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), label, TRUE, TRUE, 0);
  
  gtk_widget_show (label);
  gtk_widget_show(window);
  return TRUE;
}

void Log (const char *st)
{
  fprintf (stderr, "%s\n", st);
}
