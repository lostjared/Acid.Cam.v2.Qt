#include<iostream>


extern "C" void pixel(int x, int y, unsigned char *rgb);
extern "C" void complete();


void pixel(int x, int y, unsigned char *rgb) {
	 rgb[0] = rgb[1] = rgb[2] = 0;
}

void complete() {

}
