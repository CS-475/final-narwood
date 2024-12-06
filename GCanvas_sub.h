#ifndef GCanvas_sub_DEFINED
#define GCanvas_sub_DEFINED

#include "include/GCanvas.h"
#include "include/GBitmap.h"
#include <string>
#include <stack>

class GBitmap;
class GColor;
class GRect;
class GCanvas;
class GMatrix;

class GCanvas_sub : public GCanvas {
public:
    
    GBitmap map;
    std::stack<GMatrix> tmStack;
    int curr_y;

    
    GCanvas_sub(const GBitmap& map) {
        this->map = map;
        this->tmStack.push(GMatrix()); //could maybe speedup by making this a defualt param
    };
    /**
     *  Fill the entire canvas with the specified color, using SRC porter-duff mode.
     */
    void clear(const GColor&);

    void drawRect(const GRect&, const GPaint&);

    void drawConvexPolygon(const GPoint[], int, const GPaint&);

    /**
     *  Fill the rectangle with the color, using SRC_OVER porter-duff mode.
     *
     *  The affected pixels are those whose centers are "contained" inside the rectangle:
     *      e.g. contained == center > min_edge && center <= max_edge
     *
     *  Any area in the rectangle that is outside of the bounds of the canvas is ignored.
     */
    void fillRect(const GRect&, const GColor&);

    void save();

    void restore();

    void concat(const GMatrix& matrix);

    void drawPath(const GPath&, const GPaint&);

    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint);

    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint&);


};

/**
 *  Implemnt this, returning an instance of your subclass of GCanvas.
 */
std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap&);
    /**
     * decide when to include each pixel - whether the center is included
     * a rectangle is 4 edges - look at boundaries indpendent of space between them
     * snap edges to closest discrete step
     * avoid accessing/writing to mem outside bounds of canvas
     * clip & round - fit to boundaries and fit to pixels
     * for y=top y<bottom, for x=left x<right, addr=bm.getAddr(x,y)
     * 
     * pixel colors
     * have to scale and round - .999999*255=254, but we want 255
     * helper function to return pixel number from 4 unsigned numbers - GPixel_PackARGB
     * per pixel, must blend each color at that pos (if alpha val allows)
     * Blend(src, dst) - returns correct val for pixel color
     *      opaque src = only that color
     *      completely transparent src - only dst
     *      Rrgb = Srgb*Sa + Drgb*Da*(1-Sa) - SrcOver
     *      Ra = Sa + (1-Sa)Da
     * premul - r val is already multiplied by its alpha! don't do it again
     * correlating - colors are indp/ dep of their alphas
     * 
     * step 1 is premul, step 2 is scale + round - [r*a*255, g*a*255, b*a*255, a]
     *      assumes r,g,b, are floats 0-1
     * assuming nums are premul, new Blend becomes Rrgba = Srgba + (1-Sa)Drgba
     * R = S + (255-Sa)D/255
     * 
     * avoid touching mem if possible - expensive
     * taking away the divide will improve the time
     */

/**
 *  Implement this, drawing into the provided canvas, and returning the title of your artwork.
 */
std::string GDrawSomething(GCanvas* canvas, GISize dim);

#endif
