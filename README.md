# Acid.Cam.v2.Qt

![scr2](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/screens/screenshot.jpg "screenshot")

![scr1](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/screens/acid.cam.img1.jpg "screenshot")

Join us on Discord: https://discord.gg/kSxTe6M8

To be able to use all the filters in this program your system should have at lest 8 GB of ram.

Program requires the Visual C++ 2019 Runtime.

You can download it here: https://www.microsoft.com/en-us/download/details.aspx?id=48145

Note: On Windows 10 make sure you create a directory you have full access to for the output directories for Webcam/Video mode. For me on my Windows 10 I have created a folder at C:\ProgramOutput for the output. 
There is currently a bug that if you try to use a resolution higher than the maximum your webcam supports it will cause the file written to be corrupt. So make sure you use a resolution your webcam supports. I will try to fix this problem in future release.

Projects requires OpenCV, Qt5, libsdl2, and libacidcam are compiled and pkg-config files are configured to compile

You will need to modify the desired pro file with paths to OpenCV on Windows (no libSDL2)  for qmake for it to compile correctly.

Acid Cam distorts video to create art. It is designed to bo used with other software or on its own. There are multiple versions of the program for macOS, Linux, and Windows as well as a command line version of the program.

This project was created in hopes of providing some entertainment, or for the Glitch artist to give you a starting image/video to use with other tools.

I call the art that I create with Acid Cam an Acid Glitch, but you can call it whatever you want. It is just meant to be a tool to use with other programs to help aid in the creation of artistic images/videos.

Acid Cam's effects are created by using Acid Cam 'filters', or pieces of code that distort an image and can be combined to produce some interesting results. The project currently has over 2,000 filters to mix and match.

![AnimatedImage](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/screens/jaredpeace.gif "screenshot")

Now uses Qt5


First download GCC/Automake/Autoconf with g++ compiler and make then compile on Debian install build tools, QT5 and OpenCV development libraries via Terminal with

	$ sudo apt-get install qt5-default libopencv-dev g++ git cmake autoconf automake libtool pkg-config libsdl2-dev

Also install pkg-config

You will also need libacidcam

Download libacidcam from: https://github.com/lostjared/libacidcam

Compile and install the library libacidcam
instructions here: https://github.com/lostjared/libacidcam/blob/master/README.md

Next download this project via git clone or zip enter the src directory

For compiling with macOS and OpenCV 4 use:

	$ qmake Acid.Cam.v2.OpenCV4.pro
    
For compiling with macOS and OpenCV 3 use:
    
   	 $ qmake Acid.Cam.v2.OpenCV3.pro

For compiling on Linux with OpenCV 4 use:

	$ qmake Acid.Cam.v2.OpenCV4.Linux.pro
    
Then to build project

	$ make -j4


If you are using a high resolution the program outputs very high bitrate for MPEG-4. To be able to play it back you can load it into Final Cut and export as H.264, use Handbrake to lower the bitrate, or use ffmpeg. The command I use to compress 4K Acid Cam videos is:

    $ ffmpeg -i "input.file.mp4" -c:v libx265 -tag:v hvc1 -crf 18 output.file.mp4

or
    
    $ ffmpeg -i "input.file.mp4" -c:v libx264 -crf 18 output_file.mp4

Then Run the program

For sample videos and updates about this software view the Facebook page: http://facebook.com/AcidCam

This is an application that will allow you to generate video files/images with live video or video files. This is accomplished by mixing different 'filters' 
in different orders to produce different results. This program could be useful to you for generating basic artwork to be manipulated further. Also I have seen 
it used to create video for music videos or just for fun. 

NOTE: Some of the filters that contain either a 720 or 1080 at the end require a lot of ram if you do not have enough the program will exit.

Cross platform version of Acid Cam written in C++11 

To compile on Linux watch this video: https://youtu.be/ntsoGTglWzY

To develop C++ filters that the program will load dynamically see the example
filter in the plugins directory. The program will look for two functions:

void pixel(int x, int y, unsigned char *buffer);

void complete();

pixel is called once for every pixel in the frame and complete is called
when every pixel has been drawn. 

The output from the program video file size can be very large. This is because OpenCV records at a very high bitrate. I usually load the file into Final Cut and share as H.264.
You also could use Handbrake of FFmpeg to adjust the file size for uploading to the internet.


For the Windows version of this program requires the Visual Studio 2015 Runtime

Download it here: https://www.microsoft.com/en-us/download/details.aspx?id=48145

Currently program works best when ran in Video mode, I have not been able to test
on host os of windows or linux, just a guest virtual machine.  But in the VM webcam 
resolution is maximum of 640x480.

Screen shots of program on different Operating Systems:

Screenshot from Windows:

![ScreenShot](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/screens/acqt-ss.png?raw=true "screenshot 1")

Screenshot from Linux:

![ScreenShot](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/screens/acidcam.2.lin.jpg?raw=true "screenshot 2")

Screenshot from macOS:

![ScreenShot](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/screens/acidcam.3.osx.jpg?raw=true "screenshot 3")

