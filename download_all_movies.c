#include "ops-linux.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

gboolean download_all_movies(GtkWidget *widget,
			     GdkEvent *event,
			     gpointer data)
{
	char logstr[STRINGSIZE];
	gboolean first=TRUE;
	file_info info;
	struct stat statbuf;

	// keep compiler quiet
	widget=widget;
	event=event;
	data=data;

    if(CheckCameraOpen()==FALSE)
      return FALSE;

	Log("Changing to p0 ...");
	if (!ChangePartition(0)) return FALSE;
	Log("Changing to /DCIM/100COACH ...");
	if (!ChangeDirectory("/DCIM/100COACH")) return FALSE;
	Log("Getting directory listing ...");
	while (GetFileInfo(&info,first)) {
		first=FALSE;
		if (info.filetype==FIFILE) {
			sprintf(logstr,"found: %s (%d)",info.filename,info.filesize);
			Log(logstr);
			if (stat(info.filename,&statbuf)>=0) {
				sprintf(logstr,"%s already exists locally: skipping!",info.filename);
				Log(logstr);
			}
			else if (!DownloadFile(info.filename,info.filename)) {
				return FALSE;
			}
		}
	}
	Log("Finished downloading movies!");

	return TRUE;
}

