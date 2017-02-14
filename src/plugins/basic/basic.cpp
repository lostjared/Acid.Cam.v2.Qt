#include<iostream>

#if defined(__linux__) || defined(__APPLE__)
#define EXPORT_FUNC
#define CDECL
#else
#define EXPORT_FUNC  __declspec(dllexport)
#define CDECL __cdecl 
#endif

extern "C" EXPORT_FUNC void CDECL pixel(int x, int y, unsigned char *rgb) {
	rgb[0] = rgb[1] = rgb[2] = 0;
}

extern "C" EXPORT_FUNC void CDECL complete() {

}


