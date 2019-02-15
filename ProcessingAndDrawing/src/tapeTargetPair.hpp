#ifndef TAPETARGETPAIR_HPP
#define TAPETARGETPAIR_HPP

#include <cmath>
#include "tapeTarget.hpp"
const int XRES = 320;
const int YRES = 240;
const int ACTUAL_XRES = 1280;
const int ACTUAL_YRES = 720;
const int FOV = 53;

struct TapeTargetPair {
    TapeTarget* left;
    TapeTarget* right;
    i64 timestamp;
    bool valid;
	double angleToTarget;
    TapeTargetPair(){}
    TapeTargetPair(TapeTarget* left, TapeTarget* right, i64 timestamp) 
    {
        this->timestamp = timestamp;
        this->left = left;
        this->right = right;
        this->angleToTarget = getAngleToTarget();
    }
    TapeTargetPair(TapeTarget* left, TapeTarget* right, i64 timestamp, double angle) 
    {
        this->timestamp = timestamp;
        this->left = left;
        this->right = right;
        this->angleToTarget = angle;
    }
    TapeTargetPair(TapeTargetPair* ttp)
    {
        this->timestamp = ttp->timestamp;
        this->left = ttp->left;
        this->right = ttp->right;
        this->angleToTarget = ttp->angleToTarget;
    }
    ~TapeTargetPair() {
        delete left;
        delete right;
    }
    int GetCenterY()
    {
        return (right->center.y + left->center.y) / 2;
    }
    int GetCenterX()
    {
        return (right->center.x + left->center.x) / 2;
    }
    int GetSize()
    {
        return left->sizeOfRotatedRect() + right->sizeOfRotatedRect();
    }

    int xDistanceToPoint(cv::Point other) {
        return std::abs(GetCenterX() - other.x);
    }

    double getAngleToTarget()
    {
        double x = ((double)this->GetCenterX()/(double)XRES)*ACTUAL_XRES;
        double centerX = (XRES/2);
        return atan((x - centerX) / (XRES / (2 * tan(FOV / 2))));
    }

    static string createCSVHeader () {
        return "Timestamp,AngleToTarget";
    }

    string createCSVLine () {
        stringstream ss;
        ss << this->timestamp << ",";
        ss << this->angleToTarget;
        return ss.str();
    }
};

#endif