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

#ifdef OPS_LINUX
#define VENDOR 0x167B
#else
#define VENDOR					0x0DCA
#endif


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#pragma pack(push,1)
typedef struct camera_command {
  u8   header[4]; //"LaMS"               // 0-4
  u8   magic[4];                         // 4-8
  u32  length;                           // 8-12
  u8   flags;                            //12-13
  u8   lun;                              //13-14
  u8   data_length;                      //14-15
  u8   command;                          //15-16
  u8   unknown;                          //16-17
  u8   data[14];                         //17-31
} camera_command;

typedef struct camera_status {
  u8   header[4]; //"LaMS"               // 0-4
  u8   magic[4];                         // 4-8
  u8   dCSWDataResidue[4];               // 8-12
  u8   status;                           // 12-13
} camera_status;
#pragma pack(pop)




gboolean ControlMessageWrite(unsigned int command, char *data, int size, int timeout);
gboolean ControlMessageRead(unsigned int command, char *data, int size, int timeout);
u8 SendCommand(const camera_command* cc, camera_status* status, int timeout);
int Read(char *p_buffer, unsigned int length, int timeout);
int Write(char *p_buffer, unsigned int length, int timeout);
gboolean ReadSector(unsigned int sector, unsigned char* buffer);
void create_command(camera_command* cc, u8 flags, u8 lun, u8 data_length, u32 length, u8 command, char* data);


gboolean Open (void);
gboolean Init (void);
gboolean CheckCameraOpen (void);




#endif
