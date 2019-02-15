#include "vision.hpp"
#include "tapeTarget.hpp"
#include "tapeTargetPair.hpp"
#include <math.h>

using namespace std;

# define PI 3.14159265358979323846



//Set up constants
cv::RNG rng(12345);
cv::Scalar MY_RED (0, 0, 255);
cv::Scalar MY_BLUE (255, 0, 0);
cv::Scalar MY_GREEN (0, 255, 0);
cv::Scalar MY_PURPLE (255, 0, 255);
cv::Scalar GUIDE_DOT(255,255,0);
cv::Point TEST_POINT(160,120);
std::vector<TapeTarget*> leftTargets;
std::vector<TapeTarget*> rightTargets;
std::vector<TapeTargetPair*> validTargetPairs;

//utility functions
void copyPointData (const cv::Point &pSource, cv::Point &pTarget) {
    pTarget.x = pSource.x;
    pTarget.y = pSource.y;
}

inline int getVal (cv::Mat &img, int x, int y) {
    //img.at is (y,x) instead of (x,y)
    return img.at<cv::Vec3b>(y, x)[0];
}

inline int getSat (cv::Mat &img, int x, int y) {
    return img.at<cv::Vec3b>(y, x)[1];
}

inline int getHue (cv::Mat &img, int x, int y) {
    return img.at<cv::Vec3b>(y, x)[2];
}

void drawPoint (cv::Mat &img, cv::Point &p, cv::Scalar &color) {
    cv::circle(img, p, 4, color, 4);
}

//checks for contour validity
bool is_valid (contour_type &hull) {
    bool valid = true; //start out assuming its valid, disprove this later
    cv::RotatedRect rotatedRect = minAreaRect(hull);
    // target angle is 14.5
    // or 345.5
    // width to height ratio of tape is 2:5.5 inch
    double angle = rotatedRect.angle * -1;
    double ratio = rotatedRect.size.width / rotatedRect.size.height;
    // printf ("Ratio: %lf\n", ratio);
    // printf ("Angle: %lf\n", angle);
    // .3636363636 repeating is the target ratio
    if (angle > 7 && angle < 16 && ratio > .1 && ratio < .4) {
        // right side
        // printf("THIS IS A RIGHT CONTOUR!!!!!!!\n");
        rightTargets.push_back(new TapeTarget(hull, rotatedRect));
    } else if (angle > 74 && angle < 83 && ratio > 2.5 && ratio < 10) {
        // left side
        // printf("THIS IS A LEFT CONTOUR!!!!!!!---------------------------\n");
        leftTargets.push_back(new TapeTarget(hull, rotatedRect));
    } else {
        valid = false;
    }
    return valid;
}

TapeTargetPair* calculate(const cv::Mat &bgr, cv::Mat &processedImage, HSVMinMax hsvFilter){
    ui64 time_began = millis_since_epoch();
    //blur the image
    cv::blur(bgr, bgr, cv::Size(5,5));
    cv::Mat hsvMat;
    //convert to hsv
    cv::cvtColor(bgr, hsvMat, cv::COLOR_BGR2HSV);

    leftTargets = std::vector<TapeTarget*>();
    rightTargets = std::vector<TapeTarget*>();
    validTargetPairs = std::vector<TapeTargetPair*>();

    //store HSV values at a given test point to send back
    // int hue = getHue(hsvMat, TEST_POINT.x, TEST_POINT.y);
    // int sat = getSat(hsvMat, TEST_POINT.x, TEST_POINT.y);
    // int val = getVal(hsvMat, TEST_POINT.x, TEST_POINT.y);

    //threshold on green (light ring color)
    cv::Mat greenThreshed;
    cv::inRange(hsvMat,
                cv::Scalar(hsvFilter.minV, hsvFilter.minS, hsvFilter.minH),
                cv::Scalar(hsvFilter.maxV, hsvFilter.maxS, hsvFilter.maxH),
                greenThreshed);

    processedImage = greenThreshed.clone();
    cv::threshold (processedImage, processedImage, 0, 255, cv::THRESH_BINARY);
    cv::cvtColor(processedImage, processedImage, CV_GRAY2BGR); 
    //processedImage = bgr.clone();  

    drawPoint (processedImage, TEST_POINT, GUIDE_DOT);

    //contour detection
    vector<contour_type> contours;
    vector<cv::Vec4i> hierarchy; //throwaway, needed for function
    try {
        cv::findContours (greenThreshed, contours, hierarchy, 
            cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    }
    catch (...) { //TODO: change this to the error that occurs when there are no contours
        return nullptr;
    }

    if (contours.size() < 1) { //definitely did not find 
        return nullptr;
    }
    
    //find the largest contour in the image
    contour_type largest;
    // double largestArea = 0;
    //store the convex hulls of any valid contours
    vector<contour_type> valid_contour_hulls;
    for (int i = 0; i < (int)contours.size(); i++) {
        contour_type contour = contours[i];
        contour_type hull;
        cv::convexHull(contour, hull);
        if (is_valid(hull)) {
            valid_contour_hulls.push_back(hull);
        }
    }

    if (valid_contour_hulls.size() < 1 || leftTargets.size() < 1 || rightTargets.size() < 1) { //definitely did not find 
        return nullptr;
    }

    printf ("#Left contours: %d --- #Right contours: %d\n", (int)leftTargets.size(), (int)rightTargets.size());
    TapeTarget* rightTarget;
    TapeTarget* leftTarget;
    TapeTarget* closestTarget;
    double minDistance, dist;
    for (int j = 0; j < (int)leftTargets.size(); j++) {
        leftTarget = leftTargets[j];
        minDistance = INT_MAX;
        for (int k = 0; k < (int)rightTargets.size(); k++) {
            rightTarget = rightTargets[k];
            // if right is to the right of the left
            if (leftTarget->center.x < rightTarget->center.x)
            { 
                //find the closest right contour
                //could have issue if actual closest is not detected
                dist = leftTarget->distanceToPoint(rightTarget->center);
                if (dist < minDistance) {
                    minDistance = dist;
                    closestTarget = rightTarget;
                }
            }
        }
        validTargetPairs.push_back(new TapeTargetPair(leftTarget, closestTarget, time_began));
    }

    if (validTargetPairs.size() < 1) { //definitely did not find 
        return nullptr;
    }

    int minDistanceToCenter = INT_MAX;
    TapeTargetPair* target;
    TapeTargetPair* closestToCenter;
    for (int l = 0; l < (int)validTargetPairs.size(); l++) 
    {
        target = validTargetPairs[l];
        dist = target->xDistanceToPoint(TEST_POINT);
        if (dist < minDistanceToCenter)
        {
            minDistanceToCenter = dist;
            closestToCenter = new TapeTargetPair(target);
        }
    }

    int numContours = validTargetPairs.size();
    printf ("# Target Pairs: %d\n", numContours);
    
    if (numContours < 1) { //definitely did not find 
        return nullptr;
    }

    cv::drawContours(processedImage, valid_contour_hulls, 0, MY_GREEN, 2);

    leftTargets.erase(leftTargets.begin(), leftTargets.begin());
    rightTargets.erase(rightTargets.begin(), rightTargets.end());
    validTargetPairs.erase(validTargetPairs.begin(), validTargetPairs.end());

    return closestToCenter;
}