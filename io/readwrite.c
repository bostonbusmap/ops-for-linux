#include "io.h"

struct usb_device* m_usb_device;
usb_dev_handle *m_p_handle;

gboolean ControlMessageRead(unsigned int command, char *data, int size, int timeout) {
  int x = usb_control_msg(m_p_handle,
			  USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_INTERFACE,
			  
			  0x01,
			  command,
			  0x0000,
			  data,
			  size,
			  timeout );
  if(x<size) {
    //    fprintf(stderr,"ControlMessageRead %d %d\n",x,size);
    return FALSE;
  }
  return TRUE;
}

gboolean ControlMessageWrite(unsigned int command, char *data, int size, int timeout) {
  int x=usb_control_msg(m_p_handle,
			USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			0x01,
			command,
			0x0101,
			data,
			size,
			timeout);
  if(x<size) {
    return FALSE;
  }
  return TRUE;
}
u8 SendCommand(const camera_command* cc, camera_status* status, int timeout) {
  if (Write((char*)cc,sizeof(camera_command),timeout) == sizeof(camera_command)) {
    if ((Read((char*)status,sizeof(camera_status), timeout) == sizeof(camera_status))) {
      Log(NOTICE, "Succeeded sending command");
      
    } else {
      Log(ERROR, "Failed sending command. Status: %02x", status->status);
      return -1;
    }
  }

  return status->status;
}


int Read(char *p_buffer, unsigned int length, int timeout)
{
  int bytes_read = -1;
  
  if (m_p_handle) {
    bytes_read = usb_bulk_read(m_p_handle,READ_ENDPOINT,(char*)p_buffer,length,timeout);
  }
  //Log( "Read returns %d", bytes_read);
  
  return bytes_read;

}

int Write(char *p_buffer, unsigned int length, int timeout)
{

  int bytes_written = -1;

  if (m_p_handle) {
    bytes_written = usb_bulk_write(m_p_handle,WRITE_ENDPOINT,p_buffer,length,timeout);						
    
  }
  //Log("Write returns %d", bytes_written);
  return bytes_written;
  
}


gboolean ReadSector(unsigned int sector, unsigned char* buffer) {
  
  /*unsigned char cmd[31] = {'L','a','M','S',0x1d,0xBA,0xAB,0x1D,0x00,0x00,0x00,0x00,0x80,0x01,0x00,0x52,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};*/ // Endpoint 80
  //  unsigned char status[13];
  camera_command cc;
  camera_status status;
  create_command(&cc, 0x80, 0x01, 0x00, cpu_to_le32(512), 0x52, NULL);
  //= 512; // Size
  *(unsigned int *)(cc.data) = cpu_to_le32(sector); // Start
  
  
  if ((Write(&cc,sizeof(camera_command),TIMEOUT) == sizeof(camera_command)) &&		// Send command
      (Read(buffer,512,TIMEOUT) == 512) &&	// Read block
      (Read(&status,sizeof(camera_status),TIMEOUT) == 13) &&	// Read status
      (status.status == 0x00)) {
    return TRUE;
  } else {
    return FALSE;
  }
  

}




gboolean ReadFlash(unsigned int sector, unsigned char* buffer, unsigned int n_sectors) {
  camera_command cc;
  camera_status status;
  int size = (512*n_sectors);
  
  create_command(&cc, 0x80, 0x01, 0x00, cpu_to_le32(size), 0x52, NULL);
  
  *(u32*)(cc.data) = cpu_to_le32(size);;

  if ((Write((const char*)&cc,sizeof(camera_command), TIMEOUT) == sizeof(camera_command)) && // Send command
      (Read(buffer,size, TIMEOUT) == size) &&	// Read block
      (Read((char*)&status,sizeof(camera_status), TIMEOUT) == sizeof(camera_status)) && // Read status
      (status.status == 0x00)) {

    return TRUE;
  } else {
    return FALSE;
  }
}


//create_command is ignorant of endian-ness
void create_command(camera_command* cc, u8 flags, u8 lun, u8 data_length, u32 data_transfer_length, u8 command, char* data) {
  int zeros[4] = {0,0,0,0};
  cc->header[0] = 'L';
  cc->header[1] = 'a';
  cc->header[2] = 'M';
  cc->header[3] = 'S';
  cc->magic[0] = 0x1d;
  cc->magic[1] = 0xba;
  cc->magic[2] = 0xab;
  cc->magic[3] = 0x1d;
  cc->length = data_transfer_length;

  cc->flags = flags;
  
  cc->lun = lun;
  cc->data_length = data_length;
  cc->command = command;
  if (data == NULL) {
    memcpy(cc->data, (char*)zeros, sizeof cc->data);
  } else {
    memcpy(cc->data, data, sizeof cc->data);
  }
}
