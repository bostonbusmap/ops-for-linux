#include "ops-linux.h"
#include <string.h>


gboolean CheckCameraOpen (void) {
#ifndef DEBUG
  if (m_p_handle == NULL) {
    Log (ERROR, "Open camcorder first");
    return FALSE;
  }
#endif
  return TRUE;
}


static gboolean Unlock (void)
{
  int t;

  // this struct is always kept in little-endian format
  struct parameters
  {
    u32 index;
    u32 keydata;
  } data;

  unsigned char *keydatastr = (unsigned char *) &data.keydata;

  char ResponseKey[STRINGSIZE*16];
  char ChallengeKey[STRINGSIZE*16];
  ResponseKey[0] = '\0';
  ChallengeKey[0] = '\0';


  for (t = 0; t < 0x80; t = t + 4) {
    data.index = cpu_to_le32(0x80 + t);
    data.keydata = cpu_to_le32(0);
    if (ControlMessageWrite (0xfe01, (char*)&data.index, 4, TIMEOUT) == FALSE)	//set index to read response
    {
      Log (ERROR, "Failed at 0xfe write (set response read index)");
      return FALSE;
    }
    if (ControlMessageRead (0xff01, (char*)&data.keydata, 4, TIMEOUT) == FALSE)	//actually read 4 bytes of response
    {
      Log (ERROR, "Failed at 0xff read (read 4 bytes of response)");
      return FALSE;
    } else {
      char tempstring[STRINGSIZE];
      snprintf(tempstring, STRINGSIZE - 1, "%02x%02x%02x%02x",
	       keydatastr[0], keydatastr[1], keydatastr[2], keydatastr[3]);
      if (strlen(tempstring) + strlen(ResponseKey) >= (STRINGSIZE * 16) - 3) {
	Log(ERROR, "Not enough memory allocated for ResponseKey. This is likely a bug");
	return FALSE;
      }
      strcat(ResponseKey, tempstring);
      //CString msg;
      //msg.Format ("%02x%02x%02x%02x", keydatastr[0], keydatastr[1], keydatastr[2], keydatastr[3]);
      //ResponseKey = ResponseKey + msg;
      
      if ((t / 4) % 8 == 7) {
	strcat(ResponseKey, "\r\n");
      }
    }

    data.index = cpu_to_le32(t);
    if (ControlMessageWrite (0xfa01, (char*)&data.index, 8, TIMEOUT) == FALSE) //write out index followed by keydata
    {
      Log (ERROR, "Failed at 0xfa write (return 4 bytes of response)");
      return FALSE;
    }

    //OK, lets gather the challenge while we're at it...
    data.index = cpu_to_le32(t);
    if (ControlMessageWrite (0xfe01, (char*)&data.index, 4, TIMEOUT) == FALSE)	//set index to read challenge
    {
      Log (ERROR, "Failed at 0xfe write (set challenge read index)");
      return FALSE;

    }
    if (ControlMessageRead (0xff01, (char*)&data.keydata, 4, TIMEOUT) == FALSE)	//actually read 4 bytes of challenge
    {
      Log (ERROR, "Failed at 0xff read (read 4 bytes of challenge)");
      return FALSE;
    } else {
      char tempstring[STRINGSIZE];
      snprintf(tempstring, STRINGSIZE - 1, "%02x%02x%02x%02x",
	       keydatastr[0], keydatastr[1], keydatastr[2], keydatastr[3]);
      if (strlen(tempstring) + strlen(ChallengeKey) >= (STRINGSIZE * 16) - 3) {
	Log(ERROR, "Not enough memory allocated for ChallengeKey. This is likely a bug");
	return FALSE;
      }

      strcat(ChallengeKey, tempstring);
      
      //CString msg;
      //msg.Format ("%02x%02x%02x%02x", keydatastr[0], keydatastr[1], keydatastr[2], keydatastr[3]);
      //ChallengeKey = ChallengeKey + msg;
      if ((t / 4) % 8 == 7) {
	strcat(ChallengeKey,"\r\n");
      }
    }

  }
  data.index = cpu_to_le32(0x1a0);
  if (ControlMessageWrite (0xfe01, (char*)&data.index, 4, TIMEOUT) == FALSE)
    Log (ERROR, "failed at 0xfe");
  ControlMessageRead (0xff01, (char*)&data.keydata, 4, TIMEOUT);
  Log (USEFUL, "Challenge Key: %s", ChallengeKey);
  
  Log (USEFUL, "Response Key: %s", ResponseKey);
 
  
  if (data.keydata == cpu_to_le32(1)) {
    Log (NOTICE, "succeeded in unlocking camcorder");
    return TRUE;
  }
  Log (ERROR, "failed to unlock camcorder");
  return FALSE;

}

gboolean unlock_camcorder (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  if (CheckCameraOpen () == FALSE)
    return FALSE;

  return Unlock ();

}

