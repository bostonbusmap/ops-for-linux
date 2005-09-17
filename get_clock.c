#include "ops-linux.h"

gboolean get_clock(GtkWidget* widget,
		   GdkEvent* event,
		   gpointer data) {
  unsigned short int buffer[7];
  if (CheckCameraOpen() == FALSE)
    return FALSE;
  if (ControlMessageRead(0xcc00, (char*)buffer, sizeof buffer, TIMEOUT) == FALSE) {
    Log("get_clock failed");
    return FALSE;
  }
  printf("Camera time\n");
  printf("Year: %d\n", le16_to_cpu(buffer[0]));
  printf("Month: %d\n", le16_to_cpu(buffer[1]));
  printf("Day: %d\n", le16_to_cpu(buffer[2]));
  printf("%02d:%02d:%02d\n", le16_to_cpu(buffer[3]), le16_to_cpu(buffer[4]), le16_to_cpu(buffer[5]));
  printf("Day of week: %02d\n", le16_to_cpu(buffer[6]));

  return TRUE;
}
