# Acid.Cam.v2.Qt

![AnimatedImage](http://lostsidedead.biz/gif/jaredpeace.gif "screenshot")

This is an application that will allow you to generate video files/images with live video or video files. This is accomplished by mixing different 'filters' 
in different orders to produce different results. This program could be useful to you for generating basic artwork to be manipulated further. Also I have seen 
it used to create video for music videos or just for fun. 

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

Screen shots of program on different Operating Systems:

Screenshot from Windows:

![ScreenShot](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/acidcam.1.win.jpg?raw=true "screenshot 1")

Screenshot from Linux:

![ScreenShot](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/acidcam.2.lin.jpg?raw=true "screenshot 2")

Screenshot from macOS:

![ScreenShot](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/acidcam.3.osx.jpg?raw=true "screenshot 3")

