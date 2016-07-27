#if !defined(DllExport)
  #ifdef WIN32
    #ifdef PIX_DLL_EXPORT
      #define DllExport __declspec(dllexport)
    #elif USE_STATIC_LIB
      #define DllExport
    #else
      #define DllExport __declspec(dllimport)
    #endif
    #pragma warning(disable: 4251) // at least temporarily to not be swamped by these warnings - TO DO: check for real fix
  #else
    #define DllExport
  #endif
#endif
