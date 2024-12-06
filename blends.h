#include "include/GPixel.h"
#include "include/GColor.h"
#include "include/GBitmap.h"
#include "include/GRect.h"
#include "include/GBlendMode.h"
#include <iostream>

inline GPixel GColorToGPixel(GColor color) {
    float a = color.a;
    int r = (int)(0.5 + color.r*a*255), g = (int)(0.5 + color.g*a*255), b = (int)(0.5 + color.b*a*255);
    int int_a = (int)(0.5 + a*255);
    //std::cout << "a: " << int_a << ", r: " << r << ", g: " << g << ", b: " << b << "\n" << std::flush;
    return GPixel_PackARGB(int_a, r, g, b);
}

inline GColor GPixelToGColor(GPixel p) {
    return {GPixel_GetR(p), GPixel_GetG(p), GPixel_GetB(p), GPixel_GetA(p)};
}

inline int divBy255(int n) {
    //credit to https://stackoverflow.com/questions/35285324/how-to-divide-16-bit-integer-by-255-with-using-sse for this equation
    return (n + 1 + (n >> 8)) >> 8;
}

// struct optInfo {
//     bool shouldReturn;
//     GPixel rPixel;
// };

// typedef GBlendMode (*BlendOpt) (int, int);

// //these fns should only optimize for equations involving dst_a; anything not relying on dst_a should be caught before entering the loop
// //unless you're a gshader... in which case you can't see your src until you enter the loop
// inline GBlendMode clear_opt(int src_a, int dst_a) { return GBlendMode::kClear; } 
// inline GBlendMode src_opt(int src_a, int dst_a) { return GBlendMode::kSrc; }
// inline GBlendMode dst_opt(int src_a, int dst_a) { return GBlendMode::kDst; }
// inline GBlendMode srcover_opt(int src_a, int dst_a) { return GBlendMode::kSrcOver; }
// inline GBlendMode dstover_opt(int src_a, int dst_a) { return GBlendMode::kDstOver; }
// inline GBlendMode srcin_opt(int src_a, int dst_a) { return GBlendMode::kSrcIn; } //dummy returns for now
// inline GBlendMode dstin_opt(int src_a, int dst_a) { return GBlendMode::kDstIn; }
// inline GBlendMode srcout_opt(int src_a, int dst_a) { return GBlendMode::kSrcOut; }
// inline GBlendMode dstout_opt(int src_a, int dst_a) { return GBlendMode::kDstOut; }
// inline GBlendMode srcatop_opt(int src_a, int dst_a) { return GBlendMode::kSrcATop; }
// inline GBlendMode dstatop_opt(int src_a, int dst_a) { return GBlendMode::kDstATop; }
// inline GBlendMode xor_opt(int src_a, int dst_a) { return GBlendMode::kXor; }

// const BlendOpt gOpts[] = {
//     clear_opt, src_opt, dst_opt, srcover_opt, dstover_opt, srcin_opt, dstin_opt, srcout_opt, dstout_opt, srcatop_opt, dstatop_opt, xor_opt
// };

// typedef int (*BlendProc) (int, int, int, int);

// inline int clear_mode(int src, int dst, int src_a, int dst_a) { return 0; }
// inline int src_mode(int src, int dst, int src_a, int dst_a) { return src; }
// inline int dst_mode(int src, int dst, int src_a, int dst_a) { return dst; }
// inline int srcover_mode(int src, int dst, int src_a, int dst_a) { return src + divBy255((255-src_a)*dst); }
// inline int dstover_mode(int src, int dst, int src_a, int dst_a) { return dst + divBy255((255-dst_a)*src); }
// inline int srcin_mode(int src, int dst, int src_a, int dst_a) { return divBy255(dst_a*src); }
// inline int dstin_mode(int src, int dst, int src_a, int dst_a) { return divBy255(src_a*dst); }
// inline int srcout_mode(int src, int dst, int src_a, int dst_a) { return divBy255((255-dst_a)*src); }
// inline int dstout_mode(int src, int dst, int src_a, int dst_a) { return divBy255((255-src_a)*dst); }
// inline int srcatop_mode(int src, int dst, int src_a, int dst_a) { return divBy255(dst_a*src + (255-src_a)*dst); }
// inline int dstatop_mode(int src, int dst, int src_a, int dst_a) { return divBy255(src_a*dst + (255-dst_a)*src); }
// inline int xor_mode(int src, int dst, int src_a, int dst_a) { return divBy255((255-src_a)*dst + (255-dst_a)*src); }

// const BlendProc gProcs[] = {
//     clear_mode, src_mode, dst_mode, srcover_mode, dstover_mode, srcin_mode, dstin_mode, srcout_mode, dstout_mode, srcatop_mode, dstatop_mode, xor_mode
// };

// // inline GPixel blend(int src, int dst, GBlendMode blendmode) {
// //     BlendOpt findbestblend = gOpts[(int)blendmode];
// //     BlendProc blendmath = gProcs[(int)findbestblend(GPixel_GetA(src), GPixel_GetA(dst))];

// //     int src_a = GPixel_GetA(src), src_r = GPixel_GetR(src), src_g = GPixel_GetG(src), src_b = GPixel_GetB(src);
// //     int dst_a = GPixel_GetA(dst), dst_r = GPixel_GetR(dst), dst_g = GPixel_GetG(dst), dst_b = GPixel_GetB(dst);
// //     int a = blendmath(src_a, dst_a, src_a, dst_a), r = blendmath(src_r, dst_r, src_a, dst_a), g = blendmath(src_g, dst_g, src_a, dst_a), b = blendmath(src_b, dst_b, src_a, dst_a);
// //     return GPixel_PackARGB(a, r, g, b);
// // }

// inline GPixel blend(int src, int dst, GBlendMode blendmode) {
//     int src_a = GPixel_GetA(src), src_r = GPixel_GetR(src), src_g = GPixel_GetG(src), src_b = GPixel_GetB(src);
//     int dst_a = GPixel_GetA(dst), dst_r = GPixel_GetR(dst), dst_g = GPixel_GetG(dst), dst_b = GPixel_GetB(dst);

//     switch(blendmode) {
//         case GBlendMode::kClear:
//             return 0;
//             break;
//         case GBlendMode::kSrc:
//             return src;
//             break;
//         case GBlendMode::kDst:
//             return dst;
//             break;
//         case GBlendMode::kSrcOver:
//             return src + divBy255((255-src_a)*dst);
//             break;
//         case GBlendMode::kDstOver:
//             return dst + divBy255((255-dst_a)*src);
//             break;
//         case GBlendMode::kSrcIn:
//             return divBy255(dst_a*src);
//             break;
//         case GBlendMode::kDstIn:
//             return divBy255(src_a*dst);
//             break;
//         case GBlendMode::kSrcOut:
//             return divBy255((255-dst_a)*src);
//             break;
//         case GBlendMode::kDstOut:
//             return divBy255((255-src_a)*dst);
//             break;
//         case GBlendMode::kSrcATop:
//             return divBy255(dst_a*src) + divBy255((255-src_a)*dst);
//             break;
//         case GBlendMode::kDstATop:
//             return divBy255(src_a*dst) + divBy255((255-dst_a)*src);
//             break;
//         case GBlendMode::kXor:
//             return divBy255((255-src_a)*dst) + divBy255((255-dst_a)*src);
//             break;
//         default:
//             return src + divBy255((255-src_a)*dst);
//     }

//     int a = blendmath(src_a, dst_a, src_a, dst_a), r = blendmath(src_r, dst_r, src_a, dst_a), g = blendmath(src_g, dst_g, src_a, dst_a), b = blendmath(src_b, dst_b, src_a, dst_a);
//     return GPixel_PackARGB(a, r, g, b);
// }

// inline void blitRow(GPixel row[], int N, GPixel src, GBlendMode blendmode) {
//     for (int i=0; i < N; i++) {
//         GPixel dst = row[i];
//         row[i] = blend(src, dst, blendmode);
//     }
// }

// inline void blitRow(GPixel row[], int N, GPixel storage[], GBlendMode blendmode) {
//     for (int i=0; i<N; i++) {
//         GPixel dst = row[i];
//         GPixel src = storage[i];
//         row[i] = blend(src, dst, blendmode);
//     } 
// }

inline void blitRow(GPixel row[], int N, GPixel src_pixel, GBlendMode blendmode) {
    int dst_a, dst_r, dst_g, dst_b;
    int src_a, src_r, src_g, src_b;
    int a, r, g, b;

    //std::cout << "z;";

    switch (blendmode) {
        case GBlendMode::kClear:
            for (int i=0; i < N; i++) {
                row[i] = GPixel_PackARGB(0,0,0,0);
            }
            break;

        case GBlendMode::kSrc:
            for (int i=0; i < N; i++) {
                row[i] = src_pixel;
            }
            break;

        case GBlendMode::kDst:
            break;

        case GBlendMode::kSrcOver:
            src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
            if (src_a == 1) {
                //std::cout << "x;";
                for (int i=0; i < N; i++) {
                a = src_a, r = src_r, g = src_g, b = src_b;
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            } else {
                //std::cout << "y;" << N << "\n";
                for (int i=0; i < N; i++) {
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = src_a + divBy255((255-src_a)*dst_a), r = src_r + divBy255((255-src_a)*dst_r), g = src_g + divBy255((255-src_a)*dst_g), b = src_b + divBy255((255-src_a)*dst_b);
                row[i] = GPixel_PackARGB(a, r, g, b);
                }
            }
            break;

        case GBlendMode::kDstOver:
            src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
            for (int i=0; i < N; i++) {
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = dst_a + divBy255((255-dst_a)*src_a), r = dst_r + divBy255((255-dst_a)*src_r), b = dst_b + divBy255((255-dst_a)*src_b), g = dst_g + divBy255((255-dst_a)*src_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kSrcIn:
            src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
            for (int i=0; i < N; i++) {
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255(dst_a*src_a), r = divBy255(dst_a*src_r), b = divBy255(dst_a*src_b), g = divBy255(dst_a*src_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kDstIn:
            src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
            for (int i=0; i < N; i++) {
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255(src_a*dst_a), r = divBy255(src_a*dst_r), b = divBy255(src_a*dst_b), g = divBy255(src_a*dst_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kSrcOut:
            src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
            for (int i=0; i < N; i++) {
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255((255-dst_a)*src_a), r = divBy255((255-dst_a)*src_r), b = divBy255((255-dst_a)*src_b), g = divBy255((255-dst_a)*src_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kDstOut:
            src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
            for (int i=0; i < N; i++) {
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255((255-src_a)*dst_a), r = divBy255((255-src_a)*dst_r), b = divBy255((255-src_a)*dst_b), g = divBy255((255-src_a)*dst_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kSrcATop:
            src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
            for (int i=0; i < N; i++) {
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255(dst_a*src_a + (255-src_a)*dst_a), r = divBy255(dst_a*src_r + (255-src_a)*dst_r), b = divBy255(dst_a*src_b + (255-src_a)*dst_b), g = divBy255(dst_a*src_g + (255-src_a)*dst_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kDstATop:
            src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
            for (int i=0; i < N; i++) {
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255(src_a*dst_a + (255-dst_a)*src_a), r = divBy255(src_a*dst_r + (255-dst_a)*src_r), b = divBy255(src_a*dst_b + (255-dst_a)*src_b), g = divBy255(src_a*dst_g + (255-dst_a)*src_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kXor:
            src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
            for (int i=0; i < N; i++) {
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255((255-src_a)*dst_a + (255-dst_a)*src_a), r = divBy255((255-src_a)*dst_r + (255-dst_a)*src_r), b = divBy255((255-src_a)*dst_b + (255-dst_a)*src_b), g = divBy255((255-src_a)*dst_g + (255-dst_a)*src_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        default:
            src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
            for (int i=0; i < N; i++) {
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = src_a + divBy255((255-src_a)*dst_a), r = src_r + divBy255((255-src_a)*dst_r), g = src_g + divBy255((255-src_a)*dst_g), b = src_b + divBy255((255-src_a)*dst_b);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
    }
}

inline void blitRow(GPixel row[], int N, GPixel storage[], GBlendMode blendmode) {
    int dst_a, dst_r, dst_g, dst_b;
    int src_a, src_r, src_g, src_b;
    int a, r, g, b;

    switch (blendmode) {
        case GBlendMode::kClear:
            for (int i=0; i < N; i++) {
                row[i] = GPixel_PackARGB(0,0,0,0);
            }
            break;

        case GBlendMode::kSrc:
            for (int i=0; i < N; i++) {
                row[i] = storage[i];;
            }
            break;

        case GBlendMode::kDst:
            break;

        case GBlendMode::kSrcOver:
            for (int i=0; i < N; i++) {
            GPixel src_pixel = storage[i];
            src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
            GPixel dst_pixel = row[i];
            dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
            a = src_a + divBy255((255-src_a)*dst_a), r = src_r + divBy255((255-src_a)*dst_r), g = src_g + divBy255((255-src_a)*dst_g), b = src_b + divBy255((255-src_a)*dst_b);
            row[i] = GPixel_PackARGB(a, r, g, b);
            }
            
            break;

        case GBlendMode::kDstOver:
            for (int i=0; i < N; i++) {
                GPixel src_pixel = storage[i];
                src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = dst_a + divBy255((255-dst_a)*src_a), r = dst_r + divBy255((255-dst_a)*src_r), b = dst_b + divBy255((255-dst_a)*src_b), g = dst_g + divBy255((255-dst_a)*src_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kSrcIn:
            for (int i=0; i < N; i++) {
                GPixel src_pixel = storage[i];
                src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255(dst_a*src_a), r = divBy255(dst_a*src_r), b = divBy255(dst_a*src_b), g = divBy255(dst_a*src_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kDstIn:
            for (int i=0; i < N; i++) {
                GPixel src_pixel = storage[i];
                src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255(src_a*dst_a), r = divBy255(src_a*dst_r), b = divBy255(src_a*dst_b), g = divBy255(src_a*dst_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kSrcOut:
            for (int i=0; i < N; i++) {
                GPixel src_pixel = storage[i];
                src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255((255-dst_a)*src_a), r = divBy255((255-dst_a)*src_r), b = divBy255((255-dst_a)*src_b), g = divBy255((255-dst_a)*src_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kDstOut:
            for (int i=0; i < N; i++) {
                GPixel src_pixel = storage[i];
                src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255((255-src_a)*dst_a), r = divBy255((255-src_a)*dst_r), b = divBy255((255-src_a)*dst_b), g = divBy255((255-src_a)*dst_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kSrcATop:
            for (int i=0; i < N; i++) {
                GPixel src_pixel = storage[i];
                src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255(dst_a*src_a + (255-src_a)*dst_a), r = divBy255(dst_a*src_r + (255-src_a)*dst_r), b = divBy255(dst_a*src_b + (255-src_a)*dst_b), g = divBy255(dst_a*src_g + (255-src_a)*dst_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kDstATop:
            for (int i=0; i < N; i++) {
                GPixel src_pixel = storage[i];
                src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255(src_a*dst_a + (255-dst_a)*src_a), r = divBy255(src_a*dst_r + (255-dst_a)*src_r), b = divBy255(src_a*dst_b + (255-dst_a)*src_b), g = divBy255(src_a*dst_g + (255-dst_a)*src_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        case GBlendMode::kXor:
            for (int i=0; i < N; i++) {
                GPixel src_pixel = storage[i];
                src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = divBy255((255-src_a)*dst_a + (255-dst_a)*src_a), r = divBy255((255-src_a)*dst_r + (255-dst_a)*src_r), b = divBy255((255-src_a)*dst_b + (255-dst_a)*src_b), g = divBy255((255-src_a)*dst_g + (255-dst_a)*src_g);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
            break;

        default:
            for (int i=0; i < N; i++) {
                GPixel src_pixel = storage[i];
                src_a = GPixel_GetA(src_pixel), src_r = GPixel_GetR(src_pixel), src_g = GPixel_GetG(src_pixel), src_b = GPixel_GetB(src_pixel);
                GPixel dst_pixel = row[i];
                dst_a = GPixel_GetA(dst_pixel), dst_r = GPixel_GetR(dst_pixel), dst_g = GPixel_GetG(dst_pixel), dst_b = GPixel_GetB(dst_pixel);
                a = src_a + divBy255((255-src_a)*dst_a), r = src_r + divBy255((255-src_a)*dst_r), g = src_g + divBy255((255-src_a)*dst_g), b = src_b + divBy255((255-src_a)*dst_b);
                row[i] = GPixel_PackARGB(a, r, g, b);
            }
    }
}


inline GBlendMode optBySrc_a(GBlendMode blendmode, int src_a) {
    switch (blendmode) {
            case GBlendMode::kDstIn:
                if (src_a == 0) { blendmode = GBlendMode::kClear; }
                if (src_a == 1) { blendmode = GBlendMode::kDst; }
                break;
            case GBlendMode::kDstOut:
                if (src_a == 0) { blendmode = GBlendMode::kDst; }
                if (src_a == 1) { blendmode = GBlendMode::kClear; }
                break;
            case GBlendMode::kSrcOver:
                if (src_a == 0) { blendmode = GBlendMode::kDst; }
                if (src_a == 1) { blendmode = GBlendMode::kSrc; }
                break;
            case GBlendMode::kSrcATop:
                if (src_a == 0) { blendmode = GBlendMode::kDst; }
                break;
            case GBlendMode::kXor:
                if (src_a == 0) { blendmode = GBlendMode::kDst; }
                break;
        }

        return blendmode;
}

//want: fn defns where you can pass in src, dst, and perform ops without unpacking
//you are doing everything 4 times
// GPixel blendedPixel(GBlendMode blendmode, GPixel src, GPixel dst) {
//     int src_a = GPixel_GetA(src); int dst_a = GPixel_GetB(dst);
//     return GPixel_PackARGB(

//     )
// }