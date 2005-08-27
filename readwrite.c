#include "ops-linux.h"

gboolean ControlMessageRead(unsigned int command, int* data, int size, int timeout) {
  int x = usb_control_msg(m_p_handle,
			  USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_INTERFACE,
			  
			  0x01,
			  command,
			  0x0000,
			  (char *)data,
			  size,
			  timeout );
  if(x<size) {
    //    fprintf(stderr,"ControlMessageRead %d %d\n",x,size);
    return FALSE;
  }
  return TRUE;
}

gboolean ControlMessageWrite(unsigned int command, int *data, int size, int timeout) {
  int x=usb_control_msg(m_p_handle,
			USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			0x01,
			command,
			0x0101,
			(char *)data,
			size,
			timeout);
  if(x<size) {
      return FALSE;
  }
  return TRUE;
}
int Read(unsigned char *p_buffer, unsigned int length, int timeout)
{
  int bytes_read = -1;
  
  if (m_p_handle) {
    bytes_read = usb_bulk_read(m_p_handle,READ_ENDPOINT,(char*)p_buffer,length,timeout);
    //    fprintf(stderr,"bytes_read: %d\n", bytes_read);
  }
  
  return bytes_read;

}

int Write(unsigned char *p_buffer, unsigned int length, int timeout)
{

  int bytes_written = -1;

  if (m_p_handle) {
    bytes_written = usb_bulk_write(m_p_handle,WRITE_ENDPOINT,(char*)p_buffer,length,timeout);						
    
  }
  
  return bytes_written;
  
}


