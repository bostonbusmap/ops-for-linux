#ifndef IO_H
#define IO_H

#include <usb.h>
#include <stdint.h>
#include <glib.h>
#include "../log.h"
#include "../endian-define.h"

extern struct usb_device* m_usb_device;
extern usb_dev_handle *m_p_handle;

#define READ_ENDPOINT			0x81 
#define WRITE_ENDPOINT			0x01

#define SHORT_TIMEOUT			500
#define TIMEOUT				4000
#define LONG_TIMEOUT			14000
#define SUPER_TIMEOUT			50000
#define TAKEPIC_TIMEOUT			30000

#define DEFAULT_CONFIGURATION	1
#define DEFAULT_INTERFACE	0
#define DEFAULT_ALT_INTERFACE	0

//#define VENDOR					0x0DCA
#define VENDOROLD 0x04c5 //Fujitsu (testmarket revision Saturn)
#define VENDOR 0x167b //Pure Digital (B1,B2,B3 revision Saturn)



typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#pragma pack(push,1)
typedef struct camera_command {
  u8   header[4]; //"LaMS"               // 0-4
  u8   dCBWTag[4];                       // 4-8
  u32  dCBWDataTransferLength; //Length  // 8-12
  u8   bmCBWFlags; //bmCBWFlags Write    //12-13
  u8   bCBWLUN;                          //13-14
  u8   bCBWCBLength;                     //14-15
  u8   CBWCB[16];                        //15-31
} camera_command;

typedef struct camera_status {
  u8   header[4]; //"LaMS"               // 0-4
  u8   dCBWTag[4];                       // 4-8
  u8   dCSWDataResidue[4];               // 8-12
  u8   bCSWStatus;                       // 12-13
} camera_status;
#pragma pack(pop)




gboolean ControlMessageWrite(unsigned int command, char *data, int size, int timeout);
gboolean ControlMessageRead(unsigned int command, char *data, int size, int timeout);
int SendCommand(const camera_command* cc, camera_status* status, int timeout);
int Read(char *p_buffer, unsigned int length, int timeout);
int Write(char *p_buffer, unsigned int length, int timeout);
gboolean ReadSector(unsigned int sector, unsigned char* buffer);
void create_command(camera_command* cc, u8 flags, gboolean bCBWLUN, char bCBWCBLength, u32 data_transfer_length, char command, char* CBWCB);


gboolean Open (void);
gboolean Init (void);
gboolean CheckCameraOpen (void);




#endif
