#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <chrono>
#include <thread>
#include <random>
#include "src/utils/Vec3.h"
#include "src/utils/Color.h"

#include "Postprocess.h"
#include "Renderer.h"


void PostProcessEffect::apply(Renderer & renderer){
    w = renderer.w;
    h = renderer.h;

    
    for (int v = 0; v < h; ++v){

        std::clog << "\rScanlines remaining: " << (renderer.h-v) << ' ' << std::flush;

        Vec3 out(0, 0, 0);
        for (int u = 0; u < w; ++u){
            fragment(
                u, v, out,
                renderer.result_image,
                renderer.image,
                renderer.screen_space_normals,
                renderer.screen_space_depth
            );
            renderer.workspace[u + v * w] = Color(out);
        }
        
    }
    

}

inline int PostProcessEffect::idx_from_coord(int u, int v) const{

    //mirror repeat

    u = u % (w * 2);
    v = v % (h * 2);

    if (u<0) u += w;
    if (v<0) v += h;

    if (u >= w-1) u = 2*w - u;
    if (v >= h-1) v = 2*h - v;

    return u + v * w;
}

inline Vec3 PostProcessEffect::sampleBuffer(const std::vector< Color > & buffer, int x, int y) const {
    auto v = buffer[idx_from_coord(x, y)];

    return Vec3(v.r / 255.0, v.g / 255.0, v.b / 255.0);
}

// not private juste in case but don't go around writing in the wrong buffers!
inline void PostProcessEffect::writeToBuffer(std::vector< Color > & buffer, int x, int y, Color elt) const {

    buffer[idx_from_coord(x, y)] = elt;
}


#define FRAGMENT fragment(int u, int v, Vec3 & OUT,const std::vector< Color > & IMAGE,const std::vector< Color > & RAW_IMAGE,const std::vector< Color > & NORMAL,const std::vector< float > & DEPTH)


void postprocess::blur::Cross_blur::FRAGMENT{
    OUT = sampleBuffer(IMAGE, u, v);

    for (int i = 1; i <= size; ++i){
        OUT += sampleBuffer(IMAGE, u+i, v);
        OUT += sampleBuffer(IMAGE, u-i, v);
        OUT += sampleBuffer(IMAGE, u, v+i);
        OUT += sampleBuffer(IMAGE, u, v-i);
    }
    OUT /= 4 * size + 1 ;
}


PostProcessEffect* postprocess::blur::Cross_blur::create(int size) {

    return new Cross_blur(size);
}