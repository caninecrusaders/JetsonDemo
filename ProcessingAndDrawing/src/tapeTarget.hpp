
struct TapeTarget {
    cv::Point center;
    contour_type contour;
    cv::RotatedRect rect;
    TapeTarget() {}
    TapeTarget(contour_type contour, cv::RotatedRect rect) {
        this->contour = contour;
        this->rect = rect;
        cv::Moments mu = cv::moments(contour, false);
        center = cv::Point2f( mu.m10/mu.m00 , mu.m01/mu.m00 );
    }
    int distanceToPoint(cv::Point other) {
        return ((center.x - other.x) * (center.x - other.x)) + ((center.y - other.y)*(center.y - other.y));
    }
    int sizeOfRotatedRect() {
        return rect.size.width * rect.size.height;
    }
};