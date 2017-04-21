#include "SleepWrapped.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void PixLib::sleep(unsigned int stime){
#ifdef WIN32
      Sleep(stime);
#else
      useconds_t ustime = (useconds_t)(1000*stime);
      usleep(ustime);
#endif
}
