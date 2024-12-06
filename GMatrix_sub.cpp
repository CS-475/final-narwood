#include "include/GMatrix.h"
#include "include/GPoint.h"
#include <cmath>
#include <iostream>


GMatrix::GMatrix() {
    fMat[0] = 1;    fMat[2] = 0;    fMat[4] = 0;
    fMat[1] = 0;    fMat[3] = 1;    fMat[5] = 0;
};

GMatrix GMatrix::Translate(float tx, float ty) {
    return GMatrix(1, 0, tx, 0, 1, ty);
};

GMatrix GMatrix::Scale(float sx, float sy) {
    return GMatrix(sx, 0, 0, 0, sy, 0);
};

GMatrix GMatrix::Rotate(float radians) { 
    return GMatrix(cos(radians), -sin(radians), 0, sin(radians), cos(radians), 0);
};

GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
    return GMatrix(a[0]*b[0]+a[2]*b[1], a[0]*b[2]+a[2]*b[3], a[0]*b[4]+a[2]*b[5]+a[4], a[1]*b[0]+a[3]*b[1], a[1]*b[2]+a[3]*b[3], a[1]*b[4]+a[3]*b[5]+a[5]);
};

nonstd::optional<GMatrix> GMatrix::invert() const {
    float a = (*this)[0], b = (*this)[1], c = (*this)[2], d = (*this)[3], e = (*this)[4], f = (*this)[5];
    float det = a*d-b*c;
    if(det == 0) { return {}; }
    return GMatrix(d/det, -c/det, (c*f-e*d)/det, -b/det, a/det, (e*b-a*f)/det);
};

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; i++) {
        float tmp = (*this)[0]*src[i].x + (*this)[2]*src[i].y + (*this)[4];
        dst[i].y = (*this)[1]*src[i].x + (*this)[3]*src[i].y + (*this)[5];
        dst[i].x = tmp;
    }
    return;
};

