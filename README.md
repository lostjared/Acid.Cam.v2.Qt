# Acid.Cam.v2.Qt

![scr2](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/screens/screenshot.jpg "screenshot")

![scr1](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/screens/acid.cam.img1.jpg "screenshot")

Welcome to the Acid Cam project, a creative tool for transforming videos into art, tailor-made for both enthusiasts and professional Glitch artists. Acid Cam offers an expansive suite of filters, enabling the production of unique visual effects known as Acid Glitches. Designed for versatility, Acid Cam can be used as a standalone application or alongside other software to enhance your artistic workflow.

Join Our Community

System Requirements

To fully enjoy Acid Cam's capabilities, your system needs at least 8 GB of RAM.
Installation of Visual C++ 2019 Runtime is required. Download it here.
Installation Notes

Windows 10 users: Ensure you create a directory (e.g., C:\ProgramOutput) with full access to avoid output issues in Webcam/Video modes.
Be mindful of your webcam's maximum resolution to prevent output corruption. We're working on a fix for future updates.
Acid Cam depends on OpenCV, Qt5, libsdl2, and libacidcam. Ensure these are compiled and configured correctly.
Modify the .pro file with OpenCV paths on Windows for successful compilation with qmake.
Getting Started

Begin by installing GCC/Automake/Autoconf along with the g++ compiler. For Debian systems, install the required build tools and libraries via:
arduino

	sudo apt-get install qt5-default libopencv-dev g++ git cmake autoconf automake libtool pkg-config libsdl2-dev
 
Don't forget to install pkg-config and libacidcam. You can find libacidcam https://github.com/lostjared/libacidcam


Compilation

After downloading Acid Cam, navigate to the src directory.
Select the appropriate qmake command based on your operating system and OpenCV version, followed by make -j4 for compilation.
Managing Output File Sizes

High-resolution settings result in large MPEG-4 bitrates. Consider using Final Cut, Handbrake, or ffmpeg for compression. For example:

Explore and Share

	ffmpeg -i "input.file.mp4" -c:v libx265 -tag:v hvc1 -crf 18 output.file.mp4
For the latest updates and sample videos, visit our Facebook page.
Acid Cam is ideal for generating foundational artwork for further manipulation, creating visuals for music videos, or simply for experimentation and fun.
Notes and Tips

Filters marked with 720 or 1080 may require significant RAM; insufficient memory will cause the program to exit.
Acid Cam is a cross-platform solution, crafted in C++11, ensuring wide accessibility and performance.
Developing Custom Filters

Interested in crafting your own filters? Check out the example in the plugins directory. Acid Cam dynamically loads filters, offering endless possibilities for customization.
Output Considerations

Due to OpenCV's high bitrate recording, output files can be quite large. Consider compressing the files before sharing online.
Platform Specifics

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''\For Windows users, Visual Studio 2015 Runtime is necessary. Download here.
Acid Cam performs optimally in Video mode, though testing on native OS environments is limited to virtual machines at this time.
We're excited to see how Acid Cam inspires your creativity, whether for professional projects or personal exploration. Your feedback and contributions are highly valued as we continue to develop and refine Acid Cam.]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘‘1

![ScreenShot](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/screens/acqt-ss.png?raw=true "screenshot 1")

Screenshot from Linux:

![ScreenShot](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/screens/acidcam.2.lin.jpg?raw=true "screenshot 2")


}|||||||||||||||
