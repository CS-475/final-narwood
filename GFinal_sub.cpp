#include "GFinal_sub.h"

std::unique_ptr<GFinal> GCreateFinal() {
    return std::make_unique<GFinal_sub>();
};