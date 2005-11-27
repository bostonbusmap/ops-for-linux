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
  Log(DEBUGGING, "last->filename == %s", last->filename);
  return TRUE;
}

gboolean DownloadAllMovies(const char* folder) { //FIXME: do array bounds checking
  file_info info;
  gboolean first = TRUE;
#ifdef _WIN32
  const char* delimiter = "\\";
#else
  const char* delimiter = "/";
#endif
  char filename[STRINGSIZE];
  char *ptr;
  strcpy(filename, folder);
  strcat(filename, delimiter);
  for (ptr = filename; *ptr; ++ptr);
  struct stat statbuf;
  
  while (GetFileInfo(&info,first)) {
    first=FALSE;
    if (info.filetype==FIFILE) {
      strcpy(ptr, info.filename);
      Log(NOTICE, "found: %s (%d)",filename,info.filesize);
      
      if (stat(filename,&statbuf)>=0) {
        Log(NOTICE, "%s already exists locally: skipping!",filename);
        
      } else if (!DownloadFile(filename,info.filename, info.filesize)) {
        return FALSE;
      }
    }
  }
  return TRUE;
}

void download_all_movies_start_thread(gpointer data);
void download_all_movies_start_thread(gpointer data) {
  char* foldername = get_download_folder();
  DownloadAllMovies(foldername);
  EnableControls(TRUE);
  free(foldername);
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
  
  if (!ChangePartition(0)) return FALSE;
  if (!ChangeDirectory("/DCIM/100COACH")) return FALSE;

  EnableControls(FALSE);
  if (!g_thread_create((GThreadFunc)download_all_movies_start_thread, NULL, FALSE, &error)) {
    Log(ERROR, "g_thread says: %s", error->message);
    //free(ts->a);
    //  free(ts);
    EnableControls(TRUE);
    return FALSE;
  }


  Log(NOTICE, "Finished downloading movies!");
  
  return TRUE;
}

