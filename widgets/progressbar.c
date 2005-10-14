#include "widgets.h"

GtkWidget *m_ctl_progress = NULL;
GtkWidget *bitrate_label = NULL;

double m_progressbar_fraction;
int m_current_bytes;
int m_previous_bytes;


gboolean watch_progress_bar (gpointer data) {
  // quiet compiler
  char tempstring[STRINGSIZE];
  double rate = (double)(m_current_bytes - m_previous_bytes) / 1024.0 * (1000.0 / REFRESH_DATA_MS);
  
  data=data;
  
  gtk_progress_bar_set_fraction(m_ctl_progress, m_progressbar_fraction);
  if (m_current_bytes && m_previous_bytes)
    snprintf(tempstring, STRINGSIZE - 1, "%f kbytes/sec", rate);
  else
    strcpy(tempstring, "");
  gtk_label_set_text(GTK_LABEL(bitrate_label), tempstring);
  m_previous_bytes = m_current_bytes;
  
  return TRUE;
}
gboolean set_progress_bar(double value) {
  //  gtk_progress_bar_set_fraction(m_ctl_progress, value);
  
  m_progressbar_fraction = value;
  //if (so_far)
    
  return TRUE;

}
gboolean set_bitrate(double bytes) {
  m_current_bytes = bytes;
  if (bytes == 0)
    m_previous_bytes = 0; //initialization
  return TRUE;
}

