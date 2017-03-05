# Acid.Cam.v2.Qt

Cross platform version of Acid Cam written in C++11

To compile on Linux watch this video: https://youtu.be/ntsoGTglWzY

To develop C++ filters that the program will load dynamically see the example
filter in the plugins directory. The program will look for two functions:

void pixel(int x, int y, unsigned char *buffer);
void complete();

pixel is called once for every pixel in the frame and complete is called
when every pixel has been drawn. 

Currently program works best when ran in Video mode, I have not been able to test
on host os of windows or linux, just a guest virtual machine.  But in the VM webcam 
resolution is maximum of 640x480.

I believe there is a bug in current version of OpenCV where on OS X if you open
a webcam capture device, close it then reopen a another it will cause the program to crash
so I will have to wait until this is resolved before I can upload a complete working macOS 
version.



