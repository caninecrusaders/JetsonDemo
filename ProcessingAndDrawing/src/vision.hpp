#ifndef VISION_HPP
#define VISION_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <iostream>
#include <utility>
#include "helper.hpp"

using namespace std;

struct VisionResultsPackage {
    i64 timestamp;
    bool valid;
	cv::Point midPoint;
    cv::Point ul, ur, ll, lr;
	double upperWidth, lowerWidth;
	double leftHeight, rightHeight, angleToTarget;
	int sampleHue, sampleSat, sampleVal;

    static string createCSVHeader () {
        return 
            "Timestamp,"
            "Valid,"
            "Midpoint_x,Midpoint_y,"
            "UL_x,UL_y,"
            "UR_x,UR_y,"
            "LL_x,LL_y,"
            "LR_x,LR_y,"
            "UpperWidth,LowerWidth,"
            "LeftHeight,RightHeight,"
            "AngleToTarget";
    }

    string createCSVLine () {
        stringstream ss;
        ss << timestamp << ",";
        ss << valid << ","; //either 0 or 1
        ss << midPoint.x << "," << midPoint.y << ",";
        ss << ul.x << "," << ul.y << ",";
        ss << ur.x << "," << ur.y << ",";
        ss << ll.x << "," << ll.y << ",";
        ss << lr.x << "," << lr.y << ",";
        ss << upperWidth << "," << lowerWidth << ",";
        ss << leftHeight << "," << rightHeight << ",";
        ss << angleToTarget;
        return ss.str();
    }
};

struct HSVMinMax {
    int minH, maxH, minS, maxS, minV, maxV;
    HSVMinMax() 
    {
        minH = 55;
        maxH = 65;
        minS = 0;
        maxS = 255;
        minV = 50;
        maxV = 255;
    }
    void setValuesFromNetworkTable(shared_ptr<NetworkTable> table)
    {
        printf("setting values from network table");
        minH = table -> GetNumber("minH", 55);
        maxH = table -> GetNumber("maxH", 65);
        minS = table -> GetNumber("minS", 0);
        maxS = table -> GetNumber("maxS", 255);
        minV = table -> GetNumber("minV", 50);
        maxV = table -> GetNumber("maxV", 255);
    }
};

typedef std::vector<cv::Point> contour_type;

const int RES_X = 320, RES_Y = 240;
// const int MIN_HUE = 55, MAX_HUE = 65;
// const int MIN_SAT = 0, MAX_SAT = 255;
// const int MIN_VAL = 50, MAX_VAL = 255;

const double
MIN_AREA = 0.001, MAX_AREA = 1000000,
MIN_WIDTH = 0, MAX_WIDTH = 100000, //rectangle width
MIN_HEIGHT = 0, MAX_HEIGHT = 100000, //rectangle height
MIN_RECT_RAT = 0.78, MAX_RECT_RAT = 1.25, //rect height / rect width
MIN_AREA_RAT = 0.9, MAX_AREA_RAT = 1.1; //cvxhull area / contour area

/**
 * Processes the raw image provided in order to determine interesting values
 * from the image. The OpenCV image pipeline (thresholding, etc) is performed on
 * processedImage so this may be sent back to the driver station laptop for 
 * debugging purposes. Results package is returned in a struct.
 * @param bgr raw image to do processing on
 * @param processedImage results of OpenCV image pipeline
 * @return results of vision processing (e.g location of target, timestamp)
 */ 
VisionResultsPackage calculate(const cv::Mat &bgr, cv::Mat &processedImage, HSVMinMax hsvFilter);
void drawOnImage (cv::Mat &img, VisionResultsPackage info);
VisionResultsPackage processingFailurePackage(ui64 time);

#endif
