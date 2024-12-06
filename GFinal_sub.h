#include "include/GFinal.h"
#include "GShader_sub.h"

class GFinal_sub : public GFinal {
    public:
    
     std::shared_ptr<GShader> createVoronoiShader(const GPoint points[], const GColor colors[], int count) override {
        return std::make_shared<GShader_sub>(points, colors, count);
    };  

    // std::shared_ptr<GShader> createSweepGradient(GPoint center, float startRadians, const GColor colors[], int count) override {
    //     return std::make_shared<GShader_sub>(center, startRadians, colors, count);
    // } ;

    std::shared_ptr<GShader> createLinearPosGradient(GPoint p0, GPoint p1, const GColor colors[], const float pos[], int count) override {
        GMatrix m = GMatrix(p1.x-p0.x, p0.y-p1.y, p0.x, p1.y-p0.y, p1.x-p0.x, p0.y);
        return std::make_shared<GShader_sub>(m, colors, pos, count);
    }
};

std::unique_ptr<GFinal> GCreateFinal();