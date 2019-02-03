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
cv::Point TEST_POINT(640,360);
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

double angleToTarget(double x, double y, double imageWidth, double FOV) {
    double centerX = (imageWidth/2) - 0.5;
    return atan((x - centerX) / (imageWidth / (2 * tan(FOV / 2))));
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
        printf("THIS IS A RIGHT CONTOUR!!!!!!!\n");
        rightTargets.push_back(new TapeTarget(hull, rotatedRect));
    } else if (angle > 74 && angle < 83 && ratio > 2.5 && ratio < 10) {
        // left side
        printf("THIS IS A LEFT CONTOUR!!!!!!!---------------------------\n");
        leftTargets.push_back(new TapeTarget(hull, rotatedRect));
    } else {
        valid = false;
    }
    return valid;
}

VisionResultsPackage calculate(const cv::Mat &bgr, cv::Mat &processedImage, HSVMinMax hsvFilter){
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
    int hue = getHue(hsvMat, TEST_POINT.x, TEST_POINT.y);
    int sat = getSat(hsvMat, TEST_POINT.x, TEST_POINT.y);
    int val = getVal(hsvMat, TEST_POINT.x, TEST_POINT.y);

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
        return processingFailurePackage(time_began, hue, sat, val);
    }

    if (contours.size() < 1) { //definitely did not find 
        return processingFailurePackage(time_began, hue, sat, val);
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
    printf ("#Left contours: %d\n", (int)leftTargets.size());
    printf ("#Right contours: %d\n", (int)rightTargets.size());
    TapeTarget* rightTarget;
    TapeTarget* leftTarget;
    TapeTarget* closestTarget;
    int minDistance, dist;
    for (int i = 0; i < (int)leftTargets.size(); i++) {
        leftTarget = leftTargets[i];
        minDistance = INT_MAX;
        for (int j = 0; j < (int)rightTargets.size(); j++) {
            rightTarget = rightTargets[j];
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
        validTargetPairs.push_back(new TapeTargetPair(*leftTarget, *closestTarget));
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
            closestToCenter = target;
        }
    }

    int numContours = validTargetPairs.size();
    printf ("# Target Pairs: %d\n", numContours);
    
    if (numContours < 1) { //definitely did not find 
        return processingFailurePackage(time_began, hue, sat, val);
    }

    
    // cv::drawContours(processedImage, largest, 0, MY_GREEN, 2);
    cv::drawContours(processedImage, valid_contour_hulls, 0, MY_GREEN, 2);
    // //get the points of corners
    // vector<cv::Point> all_points;
    // all_points.insert(largest.begin(), largest.end());

    // //find which corner is which
    // cv::Point ul (1000, 1000), ur (0, 1000), ll (1000, 0), lr (0, 0);
    // for (int i = 0; i < (int)all_points.size(); i++) {
    //     int sum = all_points[i].x + all_points[i].y;
    //     int dif = all_points[i].x - all_points[i].y;

    //     if (sum < ul.x + ul.y) {
    //         ul = all_points[i];
    //     }

    //     if (sum > lr.x + lr.y) {
    //         lr = all_points[i];
    //     }

    //     if (dif < ll.x - ll.y) {
    //         ll = all_points[i];
    //     }

    //     if (dif > ur.x - ur.y) {
    //         ur = all_points[i];
    //     }
    // } 

    //find the center of mass of the largest contour
    // cv::Moments centerMass = cv::moments(largest, true);
    // double centerX = (centerMass.m10) / (centerMass.m00);
    // double centerY = (centerMass.m01) / (centerMass.m00);
    // cv::Point center (centerX, centerY);

    // vector<contour_type> largestArr;
    // largestArr.push_back(largest);

    // double top_width = ur.x - ul.x;
    // double bottom_width = lr.x - ll.x;
    // double left_height = ll.y - ul.y;
    // double right_height = lr.y - ur.y;

    //create the results package
    VisionResultsPackage res;
    res.timestamp = time_began;
    res.valid = true;
    
    // copyPointData (ul, res.ul);
    // copyPointData (ur, res.ur);
    // copyPointData (ll, res.ll);
    // copyPointData (lr, res.lr);
    // copyPointData (center, res.midPoint);


    // res.upperWidth = top_width;
    // res.lowerWidth = bottom_width;
    // res.leftHeight = left_height;
    // res.rightHeight = right_height;
    int centerX = closestToCenter->GetCenterX();
    int centerY = closestToCenter->GetCenterY();
    printf ("CenterX: %d\nCenterY: %d\n", centerX, centerY);

    // logitech = 0.86538
    // res.angleToTarget = angleToTarget(centerX, centerY, 480, 0.86538);
    // microsoft = 0.9975 focal length
    res.angleToTarget = angleToTarget(centerX, centerY, 1280, 53);

    res.sampleHue = hue;
    res.sampleSat = sat;
    res.sampleVal = val;

    leftTargets.erase(leftTargets.begin(), leftTargets.begin());
    rightTargets.erase(rightTargets.begin(), rightTargets.end());
    validTargetPairs.erase(validTargetPairs.begin(), validTargetPairs.end());

    // delete closestToCenter;
    // delete valid_contour_hulls;
    // delete validTargetPairs;
    // delete leftTargets;
    // delete rightTargets;

    // drawOnImage (processedImage, res);
    return res;
}

void drawOnImage (cv::Mat &img, VisionResultsPackage info) {
    //draw the 4 corners on the image
    drawPoint (img, info.ul, MY_BLUE);
    drawPoint (img, info.ur, MY_RED);
    drawPoint (img, info.ll, MY_BLUE);
    drawPoint (img, info.lr, MY_RED);
    drawPoint (img, info.midPoint, MY_PURPLE);
}

VisionResultsPackage processingFailurePackage(ui64 time, int h, int s, int v) {
    VisionResultsPackage failureResult;
    failureResult.timestamp = time;
    failureResult.valid = false;
    
    copyPointData (cv::Point (-1, -1), failureResult.ul);
    copyPointData (cv::Point (-1, -1), failureResult.ur);
    copyPointData (cv::Point (-1, -1), failureResult.ll);
    copyPointData (cv::Point (-1, -1), failureResult.lr);
    copyPointData (cv::Point (-1, -1), failureResult.midPoint);

    failureResult.upperWidth = -1;
    failureResult.lowerWidth = -1;
    failureResult.leftHeight = -1;
    failureResult.rightHeight = -1;

    failureResult.sampleHue = h;
    failureResult.sampleSat = s;
    failureResult.sampleVal = v;

    return failureResult;
}


