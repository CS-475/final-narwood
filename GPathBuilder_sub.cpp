#include "include/GPathBuilder.h"


 /**
     *  Append a new contour to this path, made up of the 4 points of the specified rect,
     *  in the specified direction.
     *
     *  In either direction the contour must begin at the top-left corner of the rect.
     */
    void GPathBuilder::addRect(const GRect& rect, GPathDirection dir) {
        //TODO!! 
        float l = rect.left, t = rect.top, r = rect.right, b = rect.bottom;
        if (dir == GPathDirection::kCW) {
            (*this).moveTo({l, t});
            (*this).lineTo({r, t});
            (*this).lineTo({r, b});
            (*this).lineTo({l, b});
            (*this).lineTo({l, t});
        } else {
            (*this).moveTo({l, t});
            (*this).lineTo({l, b});
            (*this).lineTo({r, b});
            (*this).lineTo({r, t});
            (*this).lineTo({l, t});
        }
        return;
    };

    /**
     *  Append a new contour to this path with the specified polygon.
     *  Calling this is equivalent to calling moveTo(pts[0]), lineTo(pts[1..count-1]).
     */
    void GPathBuilder::addPolygon(const GPoint pts[], int count) {
        //TODO!!
        //add a check on count? <2 etc.
        (*this).moveTo(pts[0]);
        for (int i = 1; i < count; i++) {
            (*this).lineTo(pts[i]);
        }
        return;
    };

    void GPathBuilder::addCircle(GPoint center, float radius, GPathDirection dir) { 
        //TODO!!!
        
        //(*this).moveTo({1,0});

        float k = 0.551915f;
        GPoint pts[11] = {{1,-k},{k,-1},{0,-1},{-k,-1},{-1,-k},{-1,0},{-1,k},{-k,1},{0,1},{k,1},{1,k}};
        
        for (int i = 0; i < 11; i++) {
            pts[i] = {pts[i].x*radius + center.x, pts[i].y*radius + center.y};
        }
        GPoint initial_pt = {1*radius + center.x, center.y};
        //make a nice unit circle first
        this->moveTo(initial_pt);
        if (dir == GPathDirection::kCCW) {
            for (int j = 0; j < 3; j++) {
                this->cubicTo(pts[j*3], pts[j*3+1], pts[j*3+2]);
            }
            this->cubicTo(pts[9],pts[10],initial_pt);
            // (*this).cubicTo({1,k},{k,1},{0,1});
            // (*this).cubicTo({-k,1},{-1,k},{-1,0});
            // (*this).cubicTo({-1,-k},{-k,-1},{0,-1});
            // (*this).cubicTo({k,-1},{1,-k},{1,0});

        } else {
            for (int j = 2; j > -1; j--) {
                this->cubicTo(pts[j*3+4], pts[j*3+3], pts[j*3+2]);
            }
            this->cubicTo(pts[1],pts[0],initial_pt);
            // (*this).cubicTo({1,-k},{k,-1},{0,-1});
            // (*this).cubicTo({-k,-1},{-1,-k},{-1,0});
            // (*this).cubicTo({-1,k},{-k,1},{0,1});
            // (*this).cubicTo({k,1},{1,k},{1,0});
        }

        //translate and rotate to match center and radius
        //GMatrix t = GMatrix::Translate(center.x, center.y);
        // GMatrix s = GMatrix::Scale(radius, radius);
        //(*this).transform(t);

        return; 
    };