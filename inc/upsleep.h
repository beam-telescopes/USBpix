#ifndef UPSLEEP
#define UPSLEEP

namespace UPGenint {
  inline void upsleep(unsigned int time){
#ifdef CF__LINUX
		usleep(time*1000);
#else
		Sleep(time);
#endif
  }
}

namespace UPGen {
  inline void Sleep(unsigned int time){
    UPGenint::upsleep(time);
  }
}
#ifdef CF__LINUX
#define	UPG_sprintf sprintf
#define	UPG_strcpy strcpy
#define	UPG_strcat strcat
#else
#define	UPG_sprintf sprintf_s
#define	UPG_strcpy strcpy_s
#define	UPG_strcat strcat_s
#endif

#endif // UPSLEEP

