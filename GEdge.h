#ifndef GEdge_DEFINED
#define GEdge_DEFINED

#include "include/GPoint.h"
#include "GPolygon.h"
#include <iostream>

struct GEdge {

    float m, b;
    int top, bottom, left, right;
    int dir;
    int curr_y = 0;

    GEdge(GPoint p0, GPoint p1) {
        this->m = (p1.x-p0.x)/(p1.y-p0.y);
        this->b = p0.x - this->m*p0.y;
        this->top = int(std::min(p0.y, p1.y) + 0.5);
        this->bottom = int(std::max(p0.y, p1.y) + 0.5);
        this->left = int(std::min(p0.x, p1.x) + 0.5);
        this->right = int(std::max(p0.x, p1.x) + 0.5);
        this->dir = p0.y < p1.y ? 1 : -1;
    }

    GEdge(float top, float bottom, float xint, int direc) {
        this->m = 0;
        this->b = int(xint + 0.5);
        this->top = int(top + 0.5);
        this->bottom = int(bottom + 0.5);
        this->left = int(xint + 0.5);
        this->right = int(xint + 0.5);
        this->dir = direc;
    }
    
    bool isUseful() { return top < bottom; }

    int xint(float y) {
        //x=my+b
        return int((m*y + b) + 0.5);
    }

    int yint(float x) {
        //y=(x-b)/m
        return int((x-b)/m + 0.5);
    }

    // int dir() { 
    //     if (m > 0) { return -1; }
    //     else if (m < 0) { return 1; }
    //     else { return 0; }
    // }

    bool isActive(int y) {
        return (top <= y && y < bottom); //should be strictly < bottom?
    }

    std::vector<GEdge> adjustEdgeBounds(int width, int height) {
        std::vector<GEdge> newEdges;
        //if (right < 0 || bottom < 0 || left > width || top > height) { return newEdges; } //covers all cases where edge is completely off canvas
        if (top < 0) {
            top = 0;
            if (m > 0) { right = this->xint(0); } 
            else { left = this->xint(0); }
        }
        if (top > height - 1){
            top = height - 1;
            if (m > 0) { right = this->xint(height - 1); }
            else { left = this->xint(height - 1); }
        }
        if (bottom > height-1) { //> or >=?
            bottom = height-1;
            if (m > 0) { left = this->xint(height-1); }
            else { right = this->xint(height-1); }
        }
        if (bottom < 0) {
            bottom = 0;
            if (m > 0) { left = this->xint(0); }
            else { right = this->xint(0); }
        }
        if (left < 0) {
            left = 0;
            if (right < 0) { right = 0; }
            GEdge e(m > 0 ? this->xint(0) < 0 ? 0 : this->xint(0), bottom, 0, this->dir : top, this->xint(0), 0, this->dir);
            if (e.isUseful()) { newEdges.push_back(e); }
        }
        if (right > width-1) { //> or >=?
            right = width-1;
            if ( left > width-1 ) { left = width-1; }
            GEdge e(m > 0 ? top, this->xint(width-1), width-1, this->dir : this->xint(width-1) < 0 ? 0 : this->xint(width-1), bottom, width-1, this->dir);
            if (e.isUseful()) { newEdges.push_back(e); }
        }
        if (this->isUseful()) {newEdges.push_back(*this);} //* may not be the right operator to use here - could be causing probs
        return newEdges;
    }

};

#endif