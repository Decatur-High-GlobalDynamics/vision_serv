//
//  FindCircle.h
//  BallTracker
//
//  Created by Aaron Hillegass on 7/10/16.
//  Copyright © 2016 Big Nerd Ranch. All rights reserved.
//

#undef check
#include <opencv/cv.h>


// Returns an array of circles of the requested hue

cv::Mat findHue(unsigned char hueLowerBound, unsigned char hueUpperBound,
                cv::Mat& hsvImage);
