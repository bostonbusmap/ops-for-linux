#include "ops-linux.h"

gboolean get_clock(GtkWidget* widget,
		   GdkEvent* event,
		   gpointer data) {
  unsigned short int buffer[7];
  if (CheckCameraOpen() == FALSE)
    return FALSE;
  if (ControlMessageRead(0xcc00, (char*)buffer, 14, TIMEOUT) == FALSE) {
    Log("get_clock failed");
    return FALSE;
  }
  printf("Camera time\n");
  printf("Year: %d\n", buffer[0]);
  printf("Month: %d\n", buffer[1]);
  printf("Day: %d\n", buffer[2]);
  printf("%02d:%02d:%02d\n", buffer[3], buffer[4], buffer[5]);
  printf("Day of week: %02d\n", buffer[6]);

  return TRUE;
}
