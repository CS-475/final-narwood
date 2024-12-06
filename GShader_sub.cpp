#include "GShader_sub.h"

std::shared_ptr<GShader> GCreateBitmapShader(GBitmap const& mapPtr, GMatrix const& localMatrix, GTileMode mode) {
    nonstd::optional<GMatrix> inv = localMatrix.invert();
    if (!inv) { return nullptr; } 
    return std::make_shared<GShader_sub>(mapPtr, localMatrix, mode);
};

std::shared_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode mode) {
    if (count < 1) { return nullptr; }
    GMatrix m = GMatrix(p1.x-p0.x, p0.y-p1.y, p0.x, p1.y-p0.y, p1.x-p0.x, p0.y);
    return std::make_shared<GShader_sub>(m, colors, count, mode);
    
};

std::shared_ptr<GShader> GCreateTriangleGradient(GPoint p0, GPoint p1, GPoint p2, const GColor colors[3], GTileMode mode) {
    GMatrix m = GMatrix(p1.x-p0.x, p2.x-p0.x, p0.x, p1.y-p0.y, p2.y-p0.y, p0.y);
    return std::make_shared<GShader_sub>(m, colors, mode);
};