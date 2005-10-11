#include "ops-linux.h"

gboolean Close (void)
{
  Log(NOTICE, "Closing device");
  if (m_usb_device) {
    if (m_p_handle) {
      usb_release_interface (m_p_handle, DEFAULT_INTERFACE);
      usb_close (m_p_handle);
      m_p_handle = NULL;
      Log (NOTICE, "Camcorder USB device closed.");
    }
  } else {
    Log(NOTICE, "No device to close");

  }
  return TRUE;
}


gboolean close_camcorder (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  return Close();
}

