#include "ops-linux.h"
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
  set_clock_struct scs;
  scs.year = cpu_to_le16(2005);
  scs.month = cpu_to_le16(9);
  scs.date = cpu_to_le16(20);
  scs.hour = cpu_to_le16(4);
  scs.minute = cpu_to_le16(5);
  scs.second = cpu_to_le16(6);
  if (CheckCameraOpen() == FALSE)
    return FALSE;
  if (ControlMessageWrite(0xcb00, (char*)&scs, sizeof scs, TIMEOUT) == FALSE) {
    Log("set_clock failed");
    return FALSE;
  }
  /*  printf("Camera time\n");
  printf("Year: %d\n", le16_to_cpu(buffer[0]));
  printf("Month: %d\n", le16_to_cpu(buffer[1]));
  printf("Day: %d\n", le16_to_cpu(buffer[2]));
  printf("%02d:%02d:%02d\n", le16_to_cpu(buffer[3]), le16_to_cpu(buffer[4]), le16_to_cpu(buffer[5]));
  printf("Day of week: %02d\n", le16_to_cpu(buffer[6]));*/

  return TRUE;
}
