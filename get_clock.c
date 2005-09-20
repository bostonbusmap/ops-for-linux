#include "ops-linux.h"
typedef struct {
  u16 year;
  u16 month;
  u16 date;
  u16 hour;
  u16 minute;
  u16 second;
  u16 day_of_week;
} get_clock_struct;

const char* months[12] = {"January", "February",
			  "March", "April",
			  "May", "June",
			  "July", "August",
			  "September", "October",
			  "November", "December" };

const char* days[7] = {"Sunday", "Monday",
		       "Tuesday", "Wednesday",
		       "Thursday", "Friday",
		       "Saturday" };
gboolean get_clock(GtkWidget* widget,
		   GdkEvent* event,
		   gpointer data) {
  
  if (CheckCameraOpen() == FALSE)
    return FALSE;
  get_clock_struct gcs;
  if (ControlMessageRead(0xcc00, (char*)&gcs, sizeof gcs, TIMEOUT) == FALSE) {
    Log("get_clock failed");
    return FALSE;
  }
  printf("Camera time\n");
  printf("Year: %d\n", le16_to_cpu(gcs.year));
  printf("Month: %s\n", months[le16_to_cpu(gcs.month) - 1]);
  printf("Day: %d\n", le16_to_cpu(gcs.date));
  printf("%02d:%02d:%02d\n", le16_to_cpu(gcs.hour), le16_to_cpu(gcs.minute), le16_to_cpu(gcs.second));
  printf("Day of week: %s\n", days[le16_to_cpu(gcs.day_of_week) - 1]);

  return TRUE;
}
