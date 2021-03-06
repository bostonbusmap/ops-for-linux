#include "ops-linux.h"

#include <time.h>

typedef struct {
  u16 year;
  u16 month;
  u16 date;
  u16 hour;
  u16 minute;
  u16 second;
} set_clock_struct;

gboolean set_clock(GtkWidget* widget,
		   GdkEvent* event,
		   gpointer data) {
  widget = widget;
  event = event;
  data = data;
  set_clock_struct scs;
  time_t rawtime;
  struct tm *t;

  time(&rawtime);
  t = localtime(&rawtime);
  scs.year = cpu_to_le16(t->tm_year+1900);
  scs.month = cpu_to_le16(t->tm_mon+1);
  scs.date = cpu_to_le16(t->tm_mday);
  scs.hour = cpu_to_le16(t->tm_hour);
  scs.minute = cpu_to_le16(t->tm_min);
  scs.second = cpu_to_le16(t->tm_sec);
  if (CheckCameraOpen() == FALSE)
    return FALSE;
  if (ControlMessageWrite(0xcb00, (char*)&scs, sizeof scs, TIMEOUT) == FALSE) {
    Log(ERROR, "set_clock failed");
    return FALSE;
  }
  Log(NOTICE, "set_clock succeeded");
  MessageBox("Set Clock succeeded");
  return TRUE;
}
