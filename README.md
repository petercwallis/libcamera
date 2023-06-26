# libcamera
A variant of the raspberry pi libcamera-app for C/C++ programmers

For those of us who think CMake just encourages bad code, this is a variant of libcamera-app with a Makefile.  The libcamera-app is designed as a drop in relacement for the original pi-camera commandline calls - which it does well - but when you want to use it from code, there is no C/C++ API documentation.  I wanted to add a fiducial finder (apriltag) as a post-processing stage but...

After following the instructions for intalling libcamera-apps, the following creates an executable that finds the camera and brings up a preview window for 5 seconds:
```
make helloWorld
```
Any command-line arguments are ignored; the code is in helloWorld.c

```
make motionDetector
```
uses the provided motion_detect_stage, retrieves the result via the post-process metadata process, and prints out the result

```
make toGrey
```
sets up a low resolution channel on the camera, gets each frame as Y420 (look it up on Wikipedia) and converts it to grey-scale. An example of frame by  frame processing.

```
make apriltag
```
Once you have downloaded and installed apriltag fiducial finder, the library will be in /usr/local/lib/arm-linux-gnueabihf/libapriltag.so  This compiles apriltag_detect,c and apriltag_detect_stage.c and links it all together to detect standard41h12 tags.
