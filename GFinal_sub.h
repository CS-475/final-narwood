#include "include/GFinal.h"
#include "GShader_sub.h"

class GFinal_sub : public GFinal {
    public:
    
     std::shared_ptr<GShader> createVoronoiShader(const GPoint points[], const GColor colors[], int count) override {
        return std::make_shared<GShader_sub>(points, colors, count);
    };   
};

std::unique_ptr<GFinal> GCreateFinal();