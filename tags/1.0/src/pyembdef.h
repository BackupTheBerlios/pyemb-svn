#ifndef PYEMBDEF_H
#define PYEMBDEF_H
  #ifdef WIN32
    #ifdef PYEMB_DLL
      #define PYEMB_DECLSPEC    __declspec(dllexport)
    #else
      #define PYEMB_DECLSPEC    __declspec(dllimport)
    #endif
  #else
    #define PYEMB_DECLSPEC
  #endif
#endif
