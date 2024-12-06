#include "include/GPixel.h"
#include "include/GColor.h"
#include "include/GRect.h"

GIRect adjustRectBounds(GRect src_rect, int width, int height) {
    float l = src_rect.left, r = src_rect.right, t = src_rect.top, b = src_rect.bottom;
    GIRect rect;
    rect.left = l < 0 ? 0 : (int)(0.5 + l); //also need to account for case where l > width of canvas 
    rect.right = r > width ? width : (int)(0.5 + r);
    rect.top = t < 0 ? 0 : (int)(0.5 + t);
    rect.bottom = b > height ? height : (int)(0.5 + b);
    return rect;
}

