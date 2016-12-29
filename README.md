# vision_serv
This is C++ code that finds stuff and communicates it back to the RoboRio

## Building

You need to install libfreenect2 and OpenCV.  You need to have a
Kinect One (that's v2) and the Kinect For Windows cable (which lets
you plug it into any USB device).  You need a C++ compiler.

This initial release has no communications yet. Also, I've only tested on my Mac and the Jetson TX1.

When you run it, it will create a /tmp/color.jpg with the image from
the camera, a circle where it thinks the ball is, and a number printed
in the center of that circle that indicates how many meters it thinks
the ball is from the Kinect. (A cropped example is here in the repo.)

(It will actually fetch a few images before generating the color.jpg
-- during the first couple of captures, it will be adjusting the
exposure.)

On a Mac, this can get and process a frame four times per
second. (Less than 10% of that time is user time -- seems to be mostly
waiting on frames from the Kinect.) The heap size is a pretty constant
90MB.

On the Jetson TX1, this can get and process slightly more than a frame
per second. Oddly, on the Jetson, the libfreenect color images come in
as RGBX (BGRX is the standard for OpenCV and what appears on the mac).
The Jetson also has some rules that you need to add to /etc/udev/ so
that the program has access to the device. The rule file is in the
libfreenect2 source.  And there is a firmware update that makes the
USB 3 port (always use USB 3 with the Kinect! The mini-USB plug is USB
2.)  works properly. That patch is here:
https://devtalk.nvidia.com/default/topic/919354/jetson-tx1/usb-3-transfer-failures/post/4899105/#4899105

I also installed OpenCV 3.2 on the Jetson so that the iwrite function
was in the imgcodecs library.

Finally, all the dynamic libraries for libfreenect and libopencv get
installed in /usr/local/lib, so you will need to set your dynamic
linker search path:

	 export LD_LIBRARY_PATH=/usr/local/lib

Given that the Kinect only talk USB3 and the Raspberry Pi 3 has only
USB2 ports, this will never run on a Raspberry Pi 3.