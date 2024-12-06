#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GBitmap.h"
#include <cmath>
#include <iostream>
#include "blends.h"

class GShader_sub : public GShader {
public:
    
    GBitmap map;
    bool gradient = false;
    bool tri_gradient = false;
    bool voroni = false;
    int c_count;
    GColor colors[50];
    nonstd::optional<GMatrix> mxInv;
    nonstd::optional<GMatrix> ctmInv;
    GTileMode tilemode = GTileMode::kClamp;
    GPoint points[50];
    


    GShader_sub(GBitmap map_param, GMatrix matrix_param, GTileMode mode) { //should this one include mode?
        map = GBitmap(map_param);
        mxInv = GMatrix(matrix_param).invert();
        tilemode = mode;
    };

    GShader_sub(GMatrix matrix_param, const GColor cs[], int num_colors, GTileMode mode) {
        gradient = true;
        mxInv = GMatrix(matrix_param).invert();
        for (int i = 0; i < num_colors; i++) { colors[i] = cs[i]; }
        c_count = num_colors;
        tilemode = mode;
    };

    GShader_sub(GMatrix matrix_param, const GColor cs[], GTileMode mode) {
        tri_gradient = true;
        mxInv = GMatrix(matrix_param).invert();
        for (int i = 0; i < 3; i++) { colors[i] = cs[i]; }
        c_count = 3;
        tilemode = mode;
    };

    GShader_sub(const GPoint ps[], const GColor cs[], int count) {
        voroni = true;
        c_count = count;
        mxInv = GMatrix(1,0,0,0,1,0);
        for (int i = 0; i < count; i++) { 
            points[i] = ps[i];
            colors[i] = cs[i]; }
    };

    ~GShader_sub() { return; }

    bool isOpaque() override {
        return map.isOpaque();
    };

    bool setContext(const GMatrix& ctm) override { 
        this->ctmInv = ctm.invert();
        if ((this->ctmInv).has_value()) { return true; }
        return false; 
    };

    static float findMirrorPoint(float coord, int thresh, int upper_bound, int parity) {
        float distance = abs(coord - thresh);
        int sections = int(distance / upper_bound); //always positive
        float rem = abs(coord) - (sections)*upper_bound - thresh;
        if (sections % 2 == parity) { return rem; }
        else { return upper_bound - rem; }
    }

    static float findRepeatPoint(float coord, int thresh, int upper_bound) {
        float distance = abs(coord - thresh);
        int sections = int(distance / upper_bound); //always positive
        float rem = abs(coord) - (sections)*upper_bound - thresh;
        if (thresh == 0) { return upper_bound - rem; }
        else { return rem; }
        
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GMatrix lookup = GMatrix::Concat(mxInv.value(), ctmInv.value()); 
        GPoint pt[1]; 
        for (int i = 0; i < count; i++) { 
            if (x==0 && y == 262 && i==455) {
                count += 1;
                count -= 1;
            }
            pt[0] = {x + i + 0.5f, y + 0.5f}; //send the middle of the point
            lookup.mapPoints(pt, pt, 1); //multiply by inv matrices
            
            if (!gradient && !tri_gradient && !voroni) { //protocol for map-based shader
                if (tilemode == GTileMode::kClamp) {
                    if (pt[0].x >= map.width()) { pt[0].x = map.width() - 1; }  //clamp; drag edge color outside bm bounds
                    if (pt[0].x < 0) { pt[0].x = 0; }
                    if (pt[0].y >= map.height()) {pt[0].y = map.height() - 1; }
                    if (pt[0].y < 0) { pt[0].y = 0; }
                } else if (tilemode == GTileMode::kRepeat) {
                    //TODO!!!!!!!!!!!
                    if (pt[0].x >= map.width()) { pt[0].x = findRepeatPoint(pt[0].x, map.width(), map.width()); } 
                    if (pt[0].x < 0) { pt[0].x = findRepeatPoint(pt[0].x, 0, map.width()); }
                    if (pt[0].y >= map.height()) { pt[0].y = findRepeatPoint(pt[0].y, map.height(), map.height()); }
                    if (pt[0].y < 0) { pt[0].y = findRepeatPoint(pt[0].y, 0, map.height()); }
                } else { //kMirror
                    if (pt[0].x >= map.width()) { pt[0].x = findMirrorPoint(pt[0].x, map.width(), map.width(), 1); } 
                    if (pt[0].x < 0) { pt[0].x = findMirrorPoint(pt[0].x, 0, map.width(), 0); }
                    if (pt[0].y >= map.height()) { pt[0].y = findMirrorPoint(pt[0].y, map.height(), map.height(), 1); }
                    if (pt[0].y < 0) { pt[0].y = findMirrorPoint(pt[0].y, 0, map.height(), 0); }
                }
                
                row[i] = *(map.getAddr(int(pt[0].x), int(pt[0].y))); //use new coords to lookup bm color
            
            } else if (gradient && !voroni) { //protocol for linear gradient

                if (tilemode == GTileMode::kClamp) {
                    if (pt[0].x < 0) { pt[0].x = 0; } 
                    if (pt[0].x > 1) { pt[0].x = 1; }
                } else if (tilemode == GTileMode::kRepeat) {
                    if (pt[0].x < 0) { pt[0].x = findRepeatPoint(pt[0].x, 0, 1); } 
                    if (pt[0].x > 1) { pt[0].x = findRepeatPoint(pt[0].x, 1, 1); }
                } else { //kMirror
                    if (pt[0].x < 0) { pt[0].x = findMirrorPoint(pt[0].x, 0, 1, 0); } 
                    if (pt[0].x > 1) { pt[0].x = findMirrorPoint(pt[0].x, 1, 1, 1); }
                }
                
                int j = (int)floor(pt[0].x * (c_count-1)); //index of first color to mix
                float t = pt[0].x * (c_count-1) - j; //distance past first color on axis
                GColor c = (1-t)*colors[j] + t*colors[j+1];
                row[i] = GColorToGPixel(c);

            } else if (tri_gradient) { //protocol for tri gradient

                if (tilemode == GTileMode::kClamp) {
                    if (pt[0].x < 0) { pt[0].x = 0; } 
                    if (pt[0].x > 1) { pt[0].x = 1; }
                    if (pt[0].y < 0) { pt[0].y = 0; }
                    if (pt[0].y > 1) { pt[0].y = 1; }

                    if (pt[0].x > 1-pt[0].y) { pt[0].x = 1-pt[0].y; }
                    if (pt[0].y > 1-pt[0].x) { pt[0].y = 1-pt[0].x; }
                
                //TODO!!!!!!!!!!!!!!!
                } else if (tilemode == GTileMode::kRepeat) {
                    if (pt[0].x < 0) { pt[0].x = findRepeatPoint(pt[0].x, 0, 1); } 
                    if (pt[0].x > 1) { pt[0].x = findRepeatPoint(pt[0].x, 1, 1); }
                } else { //kMirror
                    if (pt[0].x < 0) { pt[0].x = findMirrorPoint(pt[0].x, 0, 1, 0); } 
                    if (pt[0].x > 1) { pt[0].x = findMirrorPoint(pt[0].x, 1, 1, 1); }
                }

                GColor c = pt[0].x*colors[1] + pt[0].y*colors[2] + (1-pt[0].x-pt[0].y)*colors[0];
                row[i] = GColorToGPixel(c);

            } else { //protocol for voroni
                float dist = 10000000;
                int small_i = 0;
                for (int j=0; j<c_count; j++) {
                    float curr_dist = (float)sqrt(pow((double)(points[j].x - pt[0].x), 2) + pow((double)(points[j].y-pt[0].y), 2));
                    if (curr_dist < dist) {
                        dist = curr_dist;
                        small_i = j;
                    }
                }
                row[i] = GColorToGPixel(colors[small_i]);
            }
            
        }

        return;
    };

};

class ProxyShader : public GShader {
    GShader* fRealShader;
    GMatrix  fExtraTransform;
    std::shared_ptr<GShader> fTriangleGradientShader = nullptr;

public:
    ProxyShader(GShader* shader, const GMatrix& extraTransform)
        : fRealShader(shader), fExtraTransform(extraTransform) {}

    ProxyShader(GShader* shader, const GMatrix& extraTransform, std::shared_ptr<GShader> triangleGradientShader)
        : fRealShader(shader), fExtraTransform(extraTransform), fTriangleGradientShader(triangleGradientShader) {}
    
    bool isOpaque() override { return fRealShader->isOpaque(); }

    bool setContext(const GMatrix& ctm) override {
        if (fTriangleGradientShader != nullptr) { fTriangleGradientShader->setContext(ctm); }
        return fRealShader->setContext(ctm * fExtraTransform);

    }
    
    void shadeRow(int x, int y, int count, GPixel row[]) override {

        if (fTriangleGradientShader != nullptr) {
            GPixel proxyRow[count];
            fRealShader->shadeRow(x, y, count, proxyRow);
            GPixel colorRow[count];
            fTriangleGradientShader->shadeRow(x, y, count, colorRow);

            float frac = 1/255.0f;
            for (int i=0; i<count; i++) { 
                
                int a = int(GPixel_GetA(proxyRow[i])*GPixel_GetA(colorRow[i])*frac + 0.5);
                int r = int(GPixel_GetR(proxyRow[i])*GPixel_GetR(colorRow[i])*frac + 0.5);
                int g = int(GPixel_GetG(proxyRow[i])*GPixel_GetG(colorRow[i])*frac + 0.5);
                int b = int(GPixel_GetB(proxyRow[i])*GPixel_GetB(colorRow[i])*frac + 0.5);
                row[i] = GPixel_PackARGB(a, r, g, b); 
                
            }
        } else {
            fRealShader->shadeRow(x, y, count, row);
        }
        

    }
};



/**
 *  Return a subclass of GShader that draws the specified bitmap and the local matrix.
 *  Returns null if the subclass can not be created.
 */
std::shared_ptr<GShader> GCreateBitmapShader(GBitmap const&, GMatrix const& localMatrix, GTileMode);

std::shared_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor[], int count, GTileMode);

std::shared_ptr<GShader> GCreateTriangleGradient(GPoint p0, GPoint p1, GPoint p2, const GColor colors[3], GTileMode mode);

