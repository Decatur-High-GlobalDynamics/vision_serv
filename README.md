# vision_serv
This is C++ code that finds stuff and communicates it back to the RoboRio

## Building

You need to install libfreenect2 and OpenCV.  You need to have a
Kinect One (that's v2) and the Kinect For Windows cable (which lets
you plug it into any USB device).  You need a C++ compiler.

This initial release has no communications yet. Also, I've only tested on my Mac. 

When you run it, it will create a /tmp/color.jpg with the image from
the camera, a circle where it thinks the ball is, and a number printed
in the center of that circle that indicates how many meters it thinks
the ball is from the Kinect. (A cropped example is here in the repo.)

On a Mac, this can get and process a frame four times per
second. (Less than 10% of that time is user time -- seems to be mostly
waiting on frames from the Kinect.) The heap size is a pretty constant
90MB.