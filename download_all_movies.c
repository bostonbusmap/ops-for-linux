#include "ops-linux.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

gboolean GetLastFileInfo(file_info* last) {
  gboolean first = TRUE;
  file_info info;
  while (GetFileInfo(&info, first)) {
    first = FALSE;
    *last = info;

  }
  fprintf(stderr, "last->filename == %s\n", last->filename);
  return TRUE;
}

void DownloadAllMovies() {
  char logstr[STRINGSIZE];
  file_info info;
  gboolean first = TRUE;
  struct stat statbuf;
  while (GetFileInfo(&info,first)) {
    first=FALSE;
    if (info.filetype==FIFILE) {
      sprintf(logstr,"found: %s (%d)",info.filename,info.filesize);
      Log(logstr);
      if (stat(info.filename,&statbuf)>=0) {
	sprintf(logstr,"%s already exists locally: skipping!",info.filename);
	Log(logstr);
      } else if (!DownloadFile(info.filename,info.filename, info.filesize)) {
	return;
      }
    }
  }
}

void download_all_movies_start_thread(gpointer data);
void download_all_movies_start_thread(gpointer data) {
  DownloadAllMovies();
  EnableControls(TRUE);
}




gboolean download_all_movies(GtkWidget *widget,
			     GdkEvent *event,
			     gpointer data) {
  GError* error = NULL;
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
  EnableControls(FALSE);
  if (!g_thread_create((GThreadFunc)download_all_movies_start_thread, NULL, FALSE, &error)) {
    Log(error->message);
    //free(ts->a);
    //  free(ts);
    EnableControls(TRUE);
    return FALSE;
  }


  Log("Finished downloading movies!");
  
  return TRUE;
}

