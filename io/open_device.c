#include "io.h"

static unsigned short m_vendor_id;
static unsigned short m_product_id;
static char m_manufacturer[STRINGSIZE];
static char m_product[STRINGSIZE];


gboolean Open (void) {
  gboolean success = FALSE;

  if (m_usb_device) {
    if (m_p_handle) {
      usb_close (m_p_handle);
    }

    m_p_handle = usb_open (m_usb_device); 
   
    if (m_p_handle) {  
      if (usb_set_configuration (m_p_handle, DEFAULT_CONFIGURATION) >= 0) {
	if (usb_claim_interface (m_p_handle, DEFAULT_INTERFACE) >= 0) {
	} //ignoring this because it acts weird
	
	if (usb_set_altinterface (m_p_handle, DEFAULT_ALT_INTERFACE)
	    >= 0) {
	} //ignoring this because it acts weird
	success = TRUE;
      } else {
	usb_release_interface (m_p_handle, DEFAULT_INTERFACE);
	usb_close (m_p_handle);
      }
    }
  }

  if (success == TRUE) {
    Log (NOTICE, "Connected to camcorder.");
  } else {
    Log (ERROR, "Couldn't connect to camcorder.");
  }

  return success;

}



gboolean Init (void) {
  char tmp[256];

  static int attempt = 0;
  m_usb_device = NULL;
  m_p_handle = NULL;
  
  if (m_usb_device == NULL) {
    usb_init ();

    if (usb_find_busses () < 0) {
      Log (ERROR,"Couldn't find USB bus.");
      return FALSE;
    }

    if (usb_find_devices () < 0) {
      Log (ERROR, "Couldn't find any USB devices.");
      return FALSE;
    }
  }

  struct usb_bus *p_bus = usb_get_busses ();

  Close ();			// Just in case if already connected.
  m_usb_device = NULL;
  
  while (p_bus) {
    struct usb_device *p_device = p_bus->devices;
    
    while (p_device) {
      
      fprintf(stderr, "usb device with VID==%04x PID==%04x\n", p_device->descriptor.idVendor, p_device->descriptor.iProduct);
      if (p_device->descriptor.idVendor == VENDOR) {

	usb_dev_handle *udev = usb_open (p_device);

	if (udev) {
	  m_usb_device = p_device;

	  m_vendor_id = p_device->descriptor.idVendor;
	  m_product_id = p_device->descriptor.idProduct;

	  if (p_device->descriptor.iManufacturer) {
	    if (usb_get_string_simple (udev, p_device->descriptor.iManufacturer, tmp, sizeof (tmp)) > 0) {
	      //              m_manufacturer = tmp;
	      strncpy (m_manufacturer, tmp, STRINGSIZE - 1);
	    }
	  }
	  if (p_device->descriptor.iProduct) {
	    if (usb_get_string_simple (udev, p_device->descriptor.iProduct, tmp, sizeof (tmp)) > 0) {
	      // m_product = tmp;
	      strncpy (m_product, tmp, STRINGSIZE - 1);
	    }
	  }

	  usb_close (udev);

	  /*      CString tstr;
	     tstr.Format ("Found the camcorder: %s %s, VID:%.4X PID:%.4X", m_manufacturer, m_product, m_vendor_id, m_product_id);
	     Log (tstr); */
	  Log(NOTICE, "Found the camcorder: %s %s, VID:%.4X PID:%.4X", m_manufacturer, m_product, m_vendor_id, m_product_id);
	  
	  break;
	} else {
	  Log (ERROR, "Found the camcorder, but couldn't open it.");

	  if (attempt == 0) {
	    attempt = 1;
	    Init ();
	  }
	  break;
	}

      }
      p_device = p_device->next;
    }

    if (m_usb_device) {
      break;
    }

    p_bus = p_bus->next;
  }

  if (m_usb_device == NULL) {
    Log (ERROR, "Couldn't find camcorder.");
    return FALSE;
  }

  attempt = 0;

  return TRUE;

}
gboolean CheckCameraOpen (void) {
#ifndef DEBUG
  if (m_p_handle == NULL) {
    Log (ERROR, "Open camcorder first");
    return FALSE;
  }
#endif
  
  return TRUE;
}
