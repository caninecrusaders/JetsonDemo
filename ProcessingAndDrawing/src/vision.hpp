#ifndef VISION_HPP
#define VISION_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <iostream>
#include <utility>
#include "helper.hpp"
#include "tapeTargetPair.hpp"

using namespace std;

struct HSVMinMax {
    int minH, maxH, minS, maxS, minV, maxV;
    HSVMinMax() 
    {
        minH = 60;
        maxH = 155;
        minS = 170;
        maxS = 255;
        minV = 20;
        maxV = 100;
    }
    void setValuesFromNetworkTable(shared_ptr<NetworkTable> table)
    {
        minH = table -> GetNumber("minH", minH);
        maxH = table -> GetNumber("maxH", maxH);
        minS = table -> GetNumber("minS", minS);
        maxS = table -> GetNumber("maxS", maxS);
        minV = table -> GetNumber("minV", minV);
        maxV = table -> GetNumber("maxV", maxV);
	    // printf("setting values from network table %d", minS);
    }
};

typedef std::vector<cv::Point> contour_type;

const int RES_X = 1280, RES_Y = 720;
// const int MIN_HUE = 55, MAX_HUE = 65;
// const int MIN_SAT = 0, MAX_SAT = 255;
// const int MIN_VAL = 50, MAX_VAL = 255;

/**
 * Processes the raw image provided in order to determine interesting values
 * from the image. The OpenCV image pipeline (thresholding, etc) is performed on
 * processedImage so this may be sent back to the driver station laptop for 
 * debugging purposes. Results package is returned in a struct.
 * @param bgr raw image to do processing on
 * @param processedImage results of OpenCV image pipeline
 * @return results of vision processing (e.g location of target, timestamp)
 */ 
TapeTargetPair* calculate(const cv::Mat &bgr, cv::Mat &processedImage, HSVMinMax hsvFilter);

#endif
