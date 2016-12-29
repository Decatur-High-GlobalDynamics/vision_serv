//
//  FindCircle.m
//  BallTracker
//
//  Created by Aaron Hillegass on 7/10/16.
//  Copyright © 2016 Big Nerd Ranch. All rights reserved.
//

#import "findHue.h"
#import <stdlib.h>
#include <opencv2/imgproc.hpp>

#define CLOSE_ENOUGH (4)
#define SLOPE (35)
#define ENOUGH_SATURATION (80)
#define ENOUGH_VALUE (60)

using namespace cv;

unsigned char hotness(unsigned char *colorPtr, unsigned char hueLowerBound, unsigned hueUpperBound) {
    
    unsigned char inSaturation = colorPtr[1];
    if (inSaturation < ENOUGH_SATURATION) {
        return 0;
    }
    
    unsigned char inValue = colorPtr[2];
    if (inValue < ENOUGH_VALUE) {
        return 0;
    }
    
    bool wrapped = (hueLowerBound > hueUpperBound);
    unsigned char inHue = colorPtr[0];
    
    if (wrapped) {
        if ((inHue < hueUpperBound) || (inHue > hueLowerBound))
            return 255;
    } else {
        if ((inHue < hueUpperBound) && (inHue > hueLowerBound))
            return 255;
    }
    return 0;
}

cv::Mat findHue(unsigned char hueLowerBound, unsigned char hueUpperBound,
                cv::Mat& hsvImage) {
    cv::Mat hueHotness(hsvImage.rows, hsvImage.cols, CV_8UC1);
    
    for (int currentRow = 0; currentRow < hsvImage.rows; currentRow++ ) {
        uchar *inPtr = hsvImage.ptr(currentRow);
        uchar *outPtr = hueHotness.ptr(currentRow);
        for (int currentColumn = 0; currentColumn < hsvImage.cols; currentColumn++) {
            *outPtr = hotness(inPtr, hueLowerBound, hueUpperBound);
            outPtr++;
            inPtr += 3;
        }
    }
    return hueHotness;
}

