#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <chrono>
#include <algorithm>
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

        std::clog << "\r\tScanlines remaining: " << (renderer.h-v) << ' ' << std::flush;

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

inline float PostProcessEffect::sampleBuffer(const std::vector< float > & buffer, int x, int y) const{
    return buffer[idx_from_coord(x, y)];
}

// not private juste in case but don't go around writing in the wrong buffers!
inline void PostProcessEffect::writeToBuffer(std::vector< Color > & buffer, int x, int y, Color elt) const {

    buffer[idx_from_coord(x, y)] = elt;
}





// effects


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

void postprocess::blur::Convolve::FRAGMENT{ //TODO vérifier que ça marche
    int size_y = kernel.size()/size_x;
    
    Vec3 s(0.0, 0.0, 0.0);
    for (int v_i = 0; v_i < size_x; ++v_i){
        for (int v_j = 0; v_j < size_y; ++v_j){
            
            s += sampleBuffer(IMAGE, u + (v_i - size_x/2), v + (v_j - size_y/2)) * kernel[v_i + v_j * size_x];
        }
    }
    OUT = s;
}



void postprocess::color::Contrast::FRAGMENT{
    OUT = sampleBuffer(IMAGE, u, v);
    // first clamp it
    OUT[0] = std::clamp(OUT[0], 0.0f, 1.0f);
    OUT[1] = std::clamp(OUT[1], 0.0f, 1.0f);
    OUT[2] = std::clamp(OUT[2], 0.0f, 1.0f);
    //contrast
    OUT[0] = val * (OUT[0]-0.5) + 0.5;
    OUT[1] = val * (OUT[1]-0.5) + 0.5;
    OUT[2] = val * (OUT[2]-0.5) + 0.5;
}


void postprocess::color::Value::FRAGMENT{
    OUT = sampleBuffer(IMAGE, u, v) * val;
}




void postprocess::utils::Depth::FRAGMENT{

    if (maxDepth == -1) maxDepth = *std::max_element(DEPTH.begin(), DEPTH.end());

    float res = sampleBuffer(DEPTH, u, v);

    if (res < 0){  //sky
        OUT = Vec3(0, 0, 0);
        return;
    }

    res = 1- res / maxDepth;
    OUT = Vec3(res, res, res);
}

void postprocess::utils::Normals::FRAGMENT{

    OUT = sampleBuffer(NORMAL, u, v);
}