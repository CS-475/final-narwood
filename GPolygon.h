#ifndef GPolygon_DEFINED
#define GPolygon_DEFINED

#include "include/GPoint.h"
#include "GEdge.h"
#include "include/GTypes.h"
#include "include/GCanvas.h"
#include "include/GBitmap.h"
#include <iostream>

struct GPolygon {

    std::vector<GEdge> edges;
    int i0 = 0;
    int i1 = 1;
    int width = 0;

    GPolygon(GBitmap map) {
        this->width = map.width();
    }

    GPolygon(const GPoint points[], int len, GBitmap map) {
        std::vector<GEdge> edges;
        
        for (int i = 1; i < len; i++) {
            if (points[i-1].y == points[i].y) { continue; } //throw out horizontal edges
            GEdge e = GEdge(points[i-1], points[i]); //create edge from 2 points
            if (!e.isUseful()) { continue; } //throw out edges that are too small
            //std::cout << "before adjustEdgeBounds\n" << std::flush;
            std::vector<GEdge> addEdges = e.adjustEdgeBounds(map.width(), map.height()); //create new in-bounds edges
            //std::cout << "after adjustEdgeBounds\n" << std::flush;
            if (addEdges.size() > 0) { edges.insert(edges.end(), addEdges.begin(), addEdges.end()); } //add new edges to full edge array
            //std::cout << "after edges insert\n" << std::flush;
        }

        if (points[len-1].y != points[0].y) { //last edge you don't get by sequentially passing through array
            GEdge e = GEdge(points[len-1], points[0]);
            if (e.isUseful()) {
                //std::cout << "2before adjustEdgeBounds\n" << std::flush;
                std::vector<GEdge> addEdges = e.adjustEdgeBounds(map.width(), map.height());
                //std::cout << "2after adjustEdgeBounds\n" << std::flush;
                if (addEdges.size() > 0) { edges.insert(edges.end(), addEdges.begin(), addEdges.end()); }
                //std::cout << "2after edges insert\n" << std::flush;
            }
        }
        //by now, all edges are legal, useful, and non-horizontal

        //sort edges by top val
        // std::sort(edges.begin(), edges.end(), compare);
        this->edges = edges;
        this->width = map.width();
    }
    

    void sort_edges() {
        //std::cout << "inside sort_edges\n" << std::flush;
        std::sort(edges.begin(), edges.end(), compare);
        //std::cout << "after sorting edges\n" << std::flush;
    }

    //sorting forwards

    static bool compare(GEdge e0, GEdge e1) {
        //update to sort l,r for ties at t
        if (e0.top == e1.top) {
            float x0 = e0.m*(e0.top + 0.5f) + e0.b;
            float x1 = e1.m*(e1.top + 0.5f) + e1.b;
            return x0 > x1; //gt or lt? notes say lt but they also say e0.top < e1.top
        }
        return e0.top > e1.top;
    }

    int top() { return edges.back().top; }
    int bottom() { return edges.front().bottom; }
    int left(int y) { 
        int l = std::min(edges[edges.size()-1].xint(y+0.5f), edges[edges.size()-2].xint(y+0.5f));
        if (l < 0) { 
            l = 0;
            //std::cout << "l less than 0";
        }
        return l; 
    }
    int right(int y) { 
        int r = std::max(edges[edges.size()-1].xint(y+0.5f), edges[edges.size()-2].xint(y+0.5f));
        if (r >= width) {
            r = width - 1;
            //std::cout << "r >= width";
        }
        return r; 
    }

    
    void updateIndices(int y) {
        //update to cover cases where >2 edges get dropped after changing y val
        if (edges[edges.size()-1].bottom <= y) {
            edges.pop_back();
            if (edges[edges.size()-1].bottom <= y) {
                edges.pop_back();
            }
        } else if (edges[edges.size()-2].bottom <= y) {
            edges.erase(edges.end()-2);
        }
    }

};

#endif