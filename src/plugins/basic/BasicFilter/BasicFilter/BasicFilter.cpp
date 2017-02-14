
#include "stdafx.h"
#include "BasicFilter.h"

BASICFILTER_API void pixel(int x, int y, unsigned char *rgb)
{
	rgb[0] = rgb[1] = rgb[2] = 0;
}

BASICFILTER_API void complete() {

}