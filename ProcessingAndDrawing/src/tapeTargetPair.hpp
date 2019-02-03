struct TapeTargetPair {
    TapeTarget left;
    TapeTarget right;
    TapeTargetPair(){}
    TapeTargetPair(TapeTarget left, TapeTarget right) 
    {
        this->left = left;
        this->right = right;
    }
    int GetCenterY()
    {
        return (right.center.y + left.center.y) / 2;
    }
    int GetCenterX()
    {
        return (right.center.x + left.center.x) / 2;
    }
    int GetSize()
    {
        return left.sizeOfRotatedRect() + right.sizeOfRotatedRect();
    }
    int xDistanceToPoint(cv::Point other) {
        
        return GetCenterX() - other.x;
    }
};