#include "ops-linux.h"

gboolean Close (void)
{
  Log("Closing device");
  if (m_usb_device) {
    if (m_p_handle) {
      usb_release_interface (m_p_handle, DEFAULT_INTERFACE);
      usb_close (m_p_handle);
      m_p_handle = NULL;
      Log ("Camcorder USB device closed.");
    }
  }
  return TRUE;
}


gboolean close_camcorder (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  return Close();
}

