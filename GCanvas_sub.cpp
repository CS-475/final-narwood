#include "include/GPixel.h"
#include "include/GCanvas.h"
#include "include/GPaint.h"
#include "GCanvas_sub.h"
#include "include/GTypes.h"
#include "GPolygon.h"
#include <iostream>
#include "utils.h"
#include "include/GBlendMode.h"
#include "include/GShader.h"
#include "GShader_sub.h"
#include "GPath_sub.h"
#include "include/GPathBuilder.h"
#include <cmath>

void GCanvas_sub::clear(const GColor& color) {
    GPixel pixel = GColorToGPixel(color);
    
    GPixel *row_addr = nullptr;
    int width = (this->map).width();
    for (int y = 0; y < (this->map).height(); y++) {
        row_addr = (this->map).getAddr(0, y);
        for (int x = 0; x < width; x++) {
            row_addr[x] = pixel;
        }
    }
};

void GCanvas_sub::drawRect(const GRect& rect, const GPaint& paint) {
    GPoint pts[4] = {{rect.x(), rect.y()}, {rect.x() + rect.width(), rect.y()}, {rect.x() + rect.width(), rect.y() + rect.height()}, {rect.x(), rect.y() + rect.height()}};
    GPoint *ptr = pts;
    drawConvexPolygon(ptr, 4, paint);
};

void GCanvas_sub::drawConvexPolygon(const GPoint* points, int count, const GPaint& paint) {
    
    if (count < 3) { return; }

    GBlendMode blendmode = paint.getBlendMode();

    GPixel src_pixel; int src_a;
    GShader* shader;

    if(paint.peekShader() == nullptr) { //paint with color
        src_pixel = GColorToGPixel(paint.getColor());
        src_a = GPixel_GetA(src_pixel);

        blendmode = optBySrc_a(blendmode, src_a);
        if (blendmode == GBlendMode::kDst) {return;}
    
    } else { //paint with shader
        //setContext of shader so that shadeRow will work properly
        GPaint newPaintJob = paint; //need a non-const paint to call shareShader
        shader = newPaintJob.peekShader(); //need an instance of GShader_sub to call methods on
        shader->setContext(tmStack.top());
    }

    GPoint pts[count];
    GMatrix m = (this->tmStack).top();
    (this->tmStack).top().mapPoints(pts, points, count); 
    GPolygon p = GPolygon(pts, count, map); //edges attribute is sorted, in bounds, useful
    //start here
    p.sort_edges();

    if (p.edges.size() < 2) { return; } //have to have more than one edge to shoot between

    GPixel *row_start = nullptr;
    
    for (int y = p.top(); y < p.bottom(); y++) { //loop across centers of all pixel rows
        int left = p.left(y); int right = p.right(y);
        if (left < map.width()) { //need this line for rects to work for some reason - try to ditch for speed if you can, but might not matter that much
            row_start = map.getAddr(p.left(y), y);

            if(paint.peekShader() == nullptr) {
                blitRow(row_start, right-left, src_pixel, blendmode);
            } else if (left < right) {
                GPixel storage[right-left]; 
                shader->shadeRow(left, y, right-left, storage);
                blitRow(row_start, right-left, storage, blendmode);
            }

            p.updateIndices(y);
        }
    }

    
};

void GCanvas_sub::fillRect(const GRect& rect, const GColor& color) {
    drawRect(rect, GPaint(color));
};


void GCanvas_sub::save() {
    (this->tmStack).push((this->tmStack).top()); //duplicate top element so next concat doesn't corrupt value
    return;
};

void GCanvas_sub::restore() {
    if ((this->tmStack).size() > 1) { (this->tmStack).pop(); } //if might make this slower and might need to account for case where save has not been prev called but >1 el in stack
    return;
};

void GCanvas_sub::concat(const GMatrix& matrix) {
    GMatrix tmp = (this->tmStack).top();
    (this->tmStack).pop();
    (this->tmStack).push(GMatrix::Concat(tmp, matrix));
    return;
};


void GCanvas_sub::drawPath(const GPath& path, const GPaint& paint) {
    //TODO!! handle curves in path while drawing

    GBlendMode blendmode = paint.getBlendMode();

    GPixel src_pixel; int src_a;
    GShader* shader;

    if(paint.peekShader() == nullptr) { //paint with color
        src_pixel = GColorToGPixel(paint.getColor());
        src_a = GPixel_GetA(src_pixel);

        blendmode = optBySrc_a(blendmode, src_a);
        if (blendmode == GBlendMode::kDst) {return;}
    
    } else { //paint with shader
        //setContext of shader so that shadeRow will work properly
        GPaint newPaintJob = paint; //need a non-const paint to call shareShader
        shader = newPaintJob.peekShader(); //need an instance of GShader_sub to call methods on
        shader->setContext(tmStack.top());
    }

    //apply transform matrix to all pts in path
    std::shared_ptr<GPath> trans_path = path.transform((this->tmStack).top());
    GPolygon p = makePolygon(trans_path, map);
    
    if (p.edges.size() < 2) { return; } //have to have more than one edge to shoot between
    
    GPixel *row_start = nullptr;
    
    for (int y = p.top(); y < p.bottom(); y++) { //loop across centers of all pixel rows
        auto compare = [y](GEdge e0, GEdge e1) {
            return e0.xint(y + 0.5f) > e1.xint(y + 0.5f);
        };

        int i = p.edges.size()-1; //start with last element
        int w = 0;
        int L;

        while (i >= 0 && p.edges[i].isActive(y)) {
            int x = GRoundToInt(p.edges[i].xint(y));
            if (w == 0) {
                L = x;
            }
            w += p.edges[i].dir;
            if (w==0) {
                int R = x;
                if (L < map.width() && L >= 0) { 
                    row_start = map.getAddr(L, y);
                    if (paint.peekShader() == nullptr) {
                        blitRow(row_start, R - L, src_pixel, blendmode);
                    } else if (L < R) {
                        GPixel storage[R-L];
                        shader->shadeRow(L, y, R - L, storage);
                        blitRow(row_start, R - L, storage, blendmode);
                    }
                }
            }

            if (p.edges[i].isActive(y+1)) { i -= 1; }
            else {
                p.edges.erase(p.edges.begin() + i);
                i -= 1;
            }

        }
        while(i >= 0 && p.edges[i].isActive(y+1)) { i -= 1; }
        std::sort(p.edges.begin() + i + 1, p.edges.end(), compare);

    }
    
};

/**
 *  Draw a mesh of triangles, with optional colors and/or texture-coordinates at each vertex.
 *
 *  The triangles are specified by successive triples of indices.
 *      int n = 0;
 *      for (i = 0; i < count; ++i) {
 *          point0 = vertx[indices[n+0]]
 *          point1 = verts[indices[n+1]]
 *          point2 = verts[indices[n+2]]
 *          ...
 *          n += 3
 *      }
 *
 *  If colors is not null, then each vertex has an associated color, to be interpolated
 *  across the triangle. The colors are referenced in the same way as the verts.
 *          color0 = colors[indices[n+0]]
 *          color1 = colors[indices[n+1]]
 *          color2 = colors[indices[n+2]]
 *
 *  If texs is not null, then each vertex has an associated texture coordinate, to be used
 *  to specify a coordinate in the paint's shader's space. If there is no shader on the
 *  paint, then texs[] should be ignored. It is referenced in the same way as verts and colors.
 *          texs0 = texs[indices[n+0]]
 *          texs1 = texs[indices[n+1]]
 *          texs2 = texs[indices[n+2]]
 *
 *  If both colors and texs[] are specified, then at each pixel their values are multiplied
 *  together, component by component.
 */

void GCanvas_sub::drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint) {
    
    int n = 0;

    if (colors != nullptr && (texs == nullptr || paint.peekShader() == nullptr)) {
        for (int i=0; i<count; i++) {
            const GPoint pts[3] = {verts[indices[n+0]], verts[indices[n+1]], verts[indices[n+2]]};
            const GColor cs[3] = {colors[indices[n+0]], colors[indices[n+1]], colors[indices[n+2]]}; 
            drawConvexPolygon(pts, 3, GPaint(
                GCreateTriangleGradient(pts[0], pts[1], pts[2], cs, GTileMode::kClamp)
            ));
            n += 3;
        }

    } else if (texs != nullptr && paint.peekShader() != nullptr) {
        
        for (int i=0; i<count; i++) {
            const GPoint pts[3] = {verts[indices[n+0]], verts[indices[n+1]], verts[indices[n+2]]};
            const GPoint txs[3] = {texs[indices[n+0]], texs[indices[n+1]], texs[indices[n+2]]}; 

            GMatrix P = GMatrix(pts[1].x-pts[0].x, pts[2].x-pts[0].x, pts[0].x, 
                                pts[1].y-pts[0].y, pts[2].y-pts[0].y, pts[0].y);
            GMatrix T = GMatrix(txs[1].x-txs[0].x, txs[2].x-txs[0].x, txs[0].x, 
                                txs[1].y-txs[0].y, txs[2].y-txs[0].y, txs[0].y);
            
            std::shared_ptr<GShader> proxy;
            if (colors != nullptr) {
                const GColor cs[3] = {colors[indices[n+0]], colors[indices[n+1]], colors[indices[n+2]]};
                std::shared_ptr<GShader> triShader = GCreateTriangleGradient(pts[0], pts[1], pts[2], cs, GTileMode::kClamp);
                proxy = std::make_shared<ProxyShader>(paint.peekShader(), GMatrix::Concat(P, T.invert().value()), triShader);
            } else {
                proxy = std::make_shared<ProxyShader>(paint.peekShader(), GMatrix::Concat(P, T.invert().value()));
            }
            GPaint p(proxy);
            drawConvexPolygon(pts, 3, p);
            n += 3;
        }
    }  
    return;
    
        

};

/**
 *  Draw the quad, with optional color and/or texture coordinate at each corner. Tesselate
 *  the quad based on "level":
 *      level == 0 --> 1 quad  -->  2 triangles
 *      level == 1 --> 4 quads -->  8 triangles
 *      level == 2 --> 9 quads --> 18 triangles
 *      ...
 *  The 4 corners of the quad are specified in this order:
 *      top-left --> top-right --> bottom-right --> bottom-left
 *  Each quad is triangulated on the diagonal top-right --> bottom-left
 *      0---1
 *      |  /|
 *      | / |
 *      |/  |
 *      3---2
 *
 *  colors and/or texs can be null. The resulting triangles should be passed to drawMesh(...).
 */

static GPoint lerp(const GPoint p0, const GPoint p1, float t) {
    return {(1-t)*p0.x + t*p1.x, (1-t)*p0.y + t*p1.y};
}

static GColor color_lerp(const GColor c0, const GColor c1, float t) {
    return (1-t)*c0 + t*c1;
}

void GCanvas_sub::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) {

    std::vector<GPoint> vs;
    std::vector<int> indices;
    std::vector<GPoint> ts;
    std::vector<GColor> cs;

    float tx, ty;
    GPoint top, bottom;
    GPoint tex_top, tex_bottom;
    GColor col_top, col_bot;

    for(int i=0; i<=level+1; i++) {
        tx = i/(level+1.0f);
        top = lerp(verts[0], verts[1], tx);
        bottom = lerp(verts[3], verts[2], tx);
        if (texs != nullptr) {
            tex_top = lerp(texs[0], texs[1], tx);
            tex_bottom = lerp(texs[3], texs[2], tx);
        }
        if (colors != nullptr) {
            col_top = color_lerp(colors[0], colors[1], tx);
            col_bot = color_lerp(colors[3], colors[2], tx);
        }
        for (int j=0; j<=level+1; j++) {
            ty = j/(level+1.0f);
            vs.push_back(lerp(top, bottom, ty));
            if (texs != nullptr) {ts.push_back(lerp(tex_top, tex_bottom, ty));}
            if (colors != nullptr) {cs.push_back(color_lerp(col_top, col_bot, ty));}
        }
    }

    for(int i=0; i<=int(pow(level+2,2))-(2*level+4); i+=level+2) {
        for (int j=0; j<=level; j++) {
            indices.insert(indices.end(), {i+j+1, i+j, i+j+level+2, i+j+1, i+j+level+3, i+j+level+2}); //indices of two triangles for each square
        }
    }

    drawMesh(vs.data(), cs.data(), ts.data(), 2*int(pow(level+1, 2)), indices.data(), paint);
    return;
};


std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& map) {
    return std::make_unique<GCanvas_sub>(map);
};

static void draw_cubic(GCanvas* canvas, GPoint p, const GPoint pts[4], GColor c) {
    GPathBuilder bu;
    bu.moveTo(p);
    bu.lineTo(pts[0]);
    bu.cubicTo(pts[1], pts[2], pts[3]);
    //bu.lineTo(p);
    canvas->drawPath(*bu.detach(), GPaint(c));
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    const GColor colors[] = {{1, 0, 0, 0.5}, {0, 0, 1, 0.5}};
    //const GColor colors2[] = {{0, 1, 0, 0.5}, {0, 0, 1, 0.5}};

    //GPoint cubic[4]; //GPoint c2[4]; GPoint c3[4]; GPoint c4[4];

    float k = 0.551915f;
    GPoint center = {128,128};
    float radius = 80;
    GPoint circle_pts[12] = {{1,-k},{k,-1},{0,-1},{-k,-1},{-1,-k},{-1,0},{-1,k},{-k,1},{0,1},{k,1},{1,k},{1,0}};
    for (int i = 0; i < 12; i++) {
        circle_pts[i] = {circle_pts[i].x*radius + center.x, circle_pts[i].y*radius + center.y};
    }

    GPathBuilder path;
    path.moveTo({radius + center.x, center.y});
    for (int j = 0; j < 4; j++) {
        path.cubicTo(circle_pts[j*3], circle_pts[j*3+1], circle_pts[j*3+2]);
    }
    //this->cubicTo(pts[9],pts[10],initial_pt);
    
    GPoint tmp0[7], tmp1[7], tmp2[7];
    GPoint cubic[4] = {{radius + center.x, center.y}, circle_pts[0], circle_pts[1], circle_pts[2]};
    GPoint pivot = {center.x, center.y};
    GPath::ChopCubicAt(cubic, tmp0, 0.25);
    GPath::ChopCubicAt(&tmp0[3], tmp1, 1.0f / 3);
    GPath::ChopCubicAt(&tmp1[3], tmp2, 0.5);
    draw_cubic(canvas, pivot, tmp0, {0.5,0.25,0.75,1});
    draw_cubic(canvas, pivot, tmp1, {0,1,0,1});
    draw_cubic(canvas, pivot, tmp2, {0,0,1,1});
    draw_cubic(canvas, pivot, &tmp2[3], {1,0,0,1});
    
    GPoint p0 = {50, 50};
    GPoint p1 = {150, 150};
    std::shared_ptr<GShader> shader = GCreateLinearGradient(p0, p1, colors, 2, GTileMode::kMirror);
    GPaint paint;
    paint.setShader(shader);


    // GPoint p2 = {40, 140};
    // GPoint p3 = {200, 140};
    // std::shared_ptr<GShader> shader2 = GCreateLinearGradient(p2, p3, colors2, 2, GTileMode::kMirror);
    // GPaint paint2;
    // paint2.setShader(shader2);

    //canvas->translate(-40, 30);
    canvas->drawPath(*path.detach(), paint);
    //canvas->drawPath(*path.detach(), paint2);

    const GPoint vs[] = {{90,100},{80,170},{120,160}};
    const GColor cs[] = {{1,0,0,1},{0,1,0,1},{0,0,1,1}};
    const int is[] = {0,1,2};
    canvas->drawMesh(vs, cs, nullptr, 1, is, GPaint());
    return "Half-eaten variety cheesecake";
};


