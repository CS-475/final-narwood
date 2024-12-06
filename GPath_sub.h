#include "include/GPath.h"
#include "include/GRect.h"
#include "GPolygon.h"
#include <cmath>

/**
 *  Return the bounds of all of the control-points in the path.
 *
 *  If there are no points, returns an empty rect (all zeros)
 */
GRect GPath::bounds() const {
    //TODO - revise to compute tight bounds of each curve segment
    if (this->countPoints() > 0) {
        float l = 999999, b = -999999, r = -999999, t = 999999;
        for(GPoint p : this->fPts) {
            if (p.x > r) { r = p.x; }
            if (p.x < l) { l = p.x; }
            if (p.y > b) { b = p.y; }
            if (p.y < t) { t = p.y; }
        }
        return {l,t,r,b};
    } else { return {0, 0, 0, 0}; }
};

/**
 *  Given 0 < t < 1, subdivide the src[] quadratic bezier at t into two new quadratics in dst[]
 *  such that
 *  0...t is stored in dst[0..2]
 *  t...1 is stored in dst[2..4]
 */
void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
    if (t >= 1 || t <= 0) { return; }

    GPoint chop = src[0]*(float)pow(1-t, 2) + 2*src[1]*t*(1-t) + src[2]*(float)pow(t, 2);

    GPoint c1 = (1-t)*src[0] + t*src[1];
    GPoint c2 = (1-t)*src[1] + t*src[2];
    
    dst[0] = src[0]; dst[1] = c1; dst[2] = chop; dst[3] = c2; dst[4] = src[2];
    return;
};

/**
 *  Given 0 < t < 1, subdivide the src[] cubic bezier at t into two new cubics in dst[]
 *  such that
 *  0...t is stored in dst[0..3]
 *  t...1 is stored in dst[3..6]
 */
void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
    if (t >= 1 || t <= 0) { return; }

    GPoint chop = src[0]*(float)pow(1-t, 3) + 3*src[1]*t*(float)pow(1-t, 2) + 3*(1-t)*src[2]*(float)pow(t, 2) + src[3]*(float)pow(t, 3);

    GPoint p1 = (1-t)*src[0] + t*src[1];
    GPoint p2 = (1-t)*src[1] + t*src[2];
    GPoint p3 = (1-t)*src[2] + t*src[3];

    GPoint c1 = (1-t)*p1 + t*p2;
    GPoint c2 = (1-t)*p2 + t*p3;
    
    dst[0] = src[0]; dst[1] = p1; dst[2] = c1; dst[3] = chop; dst[4] = c2; dst[5] = p3; dst[6] = src[3];
    return;
};

static float quadErrorMag(GPoint a, GPoint b, GPoint c) { 
    GPoint p = a - 2*b + c;
    return (float)sqrt(pow(p.x, 2) + pow(p.y, 2));
};

static float cubicErrorMag(GPoint a, GPoint b, GPoint c, GPoint d) { 
    GPoint p0 = a - 2*b + c;
    GPoint p1 = b - 2*c + d;
    return (float)sqrt(pow(std::max(abs(p0.x), abs(p1.x)), 2) + pow(std::max(abs(p0.y), abs(p1.y)), 2));
};

static GPolygon makePolygon(std::shared_ptr<GPath> path, GBitmap map) { 

    GPolygon p = GPolygon(map);

    GPath::Edger edger(*path);
    std::vector<GPoint> pts;
    GPoint e_pts[GPath::kMaxNextPoints];

    GPoint i_pts[GPath::kMaxNextPoints];
    GPath::Iter iter(*path);

    
    while (auto v = edger.next(e_pts)) { //while edger is still iterating
        auto iter_value = iter.next(i_pts);
            if (iter_value.has_value()) {                   
                if (iter_value.value() == GPathVerb::kMove) { //moving; make a polygon of all previous points, then "pick up" the pen
                    if (!pts.empty()) {
                        GPolygon tmp_p = GPolygon(pts.data(), pts.size(), map);
                        tmp_p.sort_edges(); //might not need this? if we sort again below
                        if (p.edges.empty()) {
                            p.edges = tmp_p.edges;
                        } else {
                            for (GEdge edge : tmp_p.edges) {
                                p.edges.push_back(edge);
                            }
                        }
                        pts.clear(); //pts have been stored in the polygon, so clear pts for the next shape you draw
                    }
                }

                int num_segs;
                float dt;
                switch (v.value()) { //what you do with the points returned by the edger is dependent on the pathverb
                    case GPathVerb::kLine:
                        pts.push_back(e_pts[0]);
                        break;
                    case GPathVerb::kQuad:
                        num_segs = (int)ceil(sqrt(quadErrorMag(e_pts[0], e_pts[1], e_pts[2])));
                        pts.push_back(e_pts[0]); //push A
                        dt = 1.0f/num_segs;
                        for (int i = 1; i < num_segs; i++) { //if num_segs > 1, push intermediate pts
                            //f(t) = (1-t)^2A + 2t(1-t)B + t^2C - finds a point along the curve
                            float t = dt*i;
                            pts.push_back((float)pow((1-t), 2)*e_pts[0] + 2*t*(1-t)*e_pts[1] + (float)pow(t, 2)*e_pts[2]);
                        }
                        pts.push_back(e_pts[2]); //push C
                        break;
                    case GPathVerb::kCubic:
                        num_segs = (int)ceil(sqrt(3*cubicErrorMag(e_pts[0], e_pts[1], e_pts[2], e_pts[3]))); 
                        pts.push_back(e_pts[0]); //push A
                        dt = 1.0f/num_segs;
                        for (int i = 1; i < num_segs; i++) { //if num_segs > 1, push intermediate pts
                            //f(t) = A(1 - t)^3 + 3Bt(1 - t)^2 + 3C(1-t)t^2 + Dt^3 - finds a point along the curve
                            float t = dt*i;
                            pts.push_back((float)pow((1-t), 3)*e_pts[0] + (float)pow(1-t, 2)*3*t*e_pts[1] + (float)pow(t, 2)*3*(1-t)*e_pts[2] + (float)pow(t, 3)*e_pts[3]);
                        }
                        pts.push_back(e_pts[3]); //push D
                        break;
                }
            }
    }

    //after looping, throw last pts in accumulator poly
    //need this bc above loop only empties pts on next kMove; loop below accounts for last shape
    if (!pts.empty()) {
        GPolygon tmp_p = GPolygon(pts.data(), pts.size(), map);
        tmp_p.sort_edges();
        if (p.edges.empty()) {
            p.edges = tmp_p.edges;
        } else {
            for (GEdge edge : tmp_p.edges) {
                p.edges.push_back(edge);
            }
        }
        pts.clear();
    }

    p.sort_edges();
    return p;
};

