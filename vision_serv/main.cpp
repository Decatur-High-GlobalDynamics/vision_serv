//
//  main.cpp
//  vision_serv
//
//  Created by Aaron Hillegass on 12/28/16.
//  Copyright © 2016 FRC4026. All rights reserved.
//

#include <iostream>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/logger.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "findHue.h"

#define PIC_COUNT (5)
#define IMG_WIDTH (1920)
#define IMG_HEIGHT (1080)
#define HUE_UPPER_BOUND (47)
#define HUE_LOWER_BOUND (25)

using namespace cv;

libfreenect2::Freenect2 freenect2;
libfreenect2::Freenect2Device *dev = 0;
libfreenect2::PacketPipeline *pipeline = 0;

bool connect() {
    if(freenect2.enumerateDevices() == 0)
    {
        std::cout << "no device connected!" << std::endl;
        return false;
    }
    std::string serial = freenect2.getDefaultDeviceSerialNumber();
    std::cout << "Found: " << serial << std::endl;
    
    pipeline = new libfreenect2::CpuPacketPipeline();
    dev = freenect2.openDevice(serial, pipeline);
    
    
    
    return true;
}

void logFrame(libfreenect2::Frame *f){
    libfreenect2::Frame::Format format = f->format;
    std::string formatString = "Unknown";
    switch (format) {
        case libfreenect2::Frame::Float:
            formatString = "Float";
            break;
        case libfreenect2::Frame::BGRX:
            formatString = "BGRX";
            break;
        case libfreenect2::Frame::RGBX:
            formatString = "RGBX";
            break;
        case libfreenect2::Frame::Gray:
            formatString = "Gray";
            break;

        default:
            break;
    }
    std::cout << "Frame:(" << f->width << "x" <<
                f->height<< ") " << formatString <<", "<< f->bytes_per_pixel << " bytes per pixel"<< std::endl;
}

float findDistance(libfreenect2::Frame *f, cv::Point2f center) {
    size_t width = f->width;
    size_t height = f->height;
    
    size_t row = (center.y / (float)IMG_HEIGHT) * height;
    size_t col = (center.x / (float)IMG_WIDTH) * width;

    float *dataPtr = (float *)f->data;
    float *pixelPtr = dataPtr + (row * width) + col;
    return *pixelPtr;
}


int main(int argc, const char * argv[]) {
    bool success = connect();
    if (!success) {
        std::cout << "Failed to connect" << std::endl;
        return -1;
    }

    int types = libfreenect2::Frame::Color |
                    libfreenect2::Frame::Depth |
                    libfreenect2::Frame::Ir;
    
    libfreenect2::SyncMultiFrameListener listener(types);
    libfreenect2::FrameMap frames;
    
    dev->setColorFrameListener(&listener);
    dev->setIrAndDepthFrameListener(&listener);
    success = dev->start();
    if (!success) {
        std::cout << "Device failed to start" << std::endl;
        return -1;
    }
    std::cout << "Device started" << std::endl;

    for (int counter = 0; counter < PIC_COUNT; counter++) { // It takes a few rounds before exposure is right
        if (!listener.waitForNewFrame(frames, 10*1000)) // 10 seconds
        {
            std::cout << "Timed out waiting for frames" << std::endl;
            return -1;
        }
	std::cout << "Got a frame" << std::endl;

        libfreenect2::Frame *color = frames[libfreenect2::Frame::Color];
        std::cout << "Color: ";
        logFrame(color);
    
        
        Mat colorMat = Mat((int)color->height, (int)color->width, CV_8UC4);
        colorMat.data = color->data;
        
        // Matrix for HSV image
        Mat hsvMatrix;

	// Oddly, the Jetson fetches RGBX images, Mac gets BGRX
	if (color->format == libfreenect2::Frame::RGBX) {
	  cvtColor(colorMat, hsvMatrix, COLOR_RGB2HSV);
	} else {
	  cvtColor(colorMat, hsvMatrix, COLOR_BGR2HSV);
	}

        
        // Matrix for hue mask
        cv::Mat hotness = findHue(HUE_LOWER_BOUND, HUE_UPPER_BOUND, hsvMatrix);

        // Find the top-level contours
        std::vector<std::vector<cv::Point> > contours;
        std::vector<Vec4i> hierarchy;
        findContours(hotness, contours, hierarchy,
                     RETR_EXTERNAL, CHAIN_APPROX_TC89_KCOS );
        
        bool found = false;
        double biggestArea = 50.0; // Ignore circles with area of less than 50 pixels
        std::vector<cv::Point> biggestContour;
        if (hierarchy.size() > 0) {
            for(int idx = 0; idx >= 0; idx = hierarchy[idx][0] ) {
                double area = contourArea(contours[idx]);
                if (area > biggestArea) {
                    biggestContour = contours[idx];
                    biggestArea = area;
                    found = true;
                    //fprintf(stderr, "contour %d: area=%.2f, {%.2f, %.2f}", idx, )
                }
            }
        }

        
        if (found) {
            cv::Point2f center;
            float radius;

            minEnclosingCircle(biggestContour,
                               center, radius);

            libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];
            logFrame(depth);

            float distance = findDistance(depth, center)/1000.0; // Convert to meters
            
            fprintf(stderr, "Circle: %.2f, %.2f: %.2f\nDistance: %.2fmm\n", center.x, center.y, radius, distance);
            if (counter == PIC_COUNT - 1) {
                cv::circle(colorMat, center, (int)radius, Scalar(255,0,0),5);
                
                int fontFace = 1;
                double fontScale = 1.8;
                int thickness = 2;
                
                char *distanceString;
                asprintf(&distanceString, "%.2f m", distance);
                
                int baseline=0;
                Size textSize = getTextSize(distanceString, fontFace,
                                            fontScale, thickness, &baseline);
                baseline += thickness;
                
                // center the text
                Point textOrg(center.x - (textSize.width/2),
                              center.y + (textSize.height/2));

                putText(colorMat, distanceString, textOrg,
                        fontFace, fontScale, Scalar(255,0,0), thickness);
                std::vector<int> compression_params;
                compression_params.push_back(IMWRITE_JPEG_QUALITY);
                compression_params.push_back(95);
                try {
                    imwrite("/tmp/color.jpg", colorMat, compression_params);
                    fprintf(stderr, "Written image\n");
                }
                catch (cv::Exception& ex) {
                    fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
                    return 1;
                }
            }
            
        } else {
            fprintf(stderr, "Circle not found\n");
        }
        listener.release(frames);
    }
    
    dev->stop();
    dev->close();
    
    return 0;
}
