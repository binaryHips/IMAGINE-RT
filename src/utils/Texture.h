
#pragma once
#include "src/utils/imageLoader.h"
#include <fstream>
#include "src/utils/Vec3.h"
#include "src/utils/Color.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <utility>
#include <cmath>
enum RepeatMode {
    TILE,
    MIRROR,
    EXTEND,
};


class Texture{

public:
    int w, h;

    virtual Color accessData(int u, int v) const = 0;
    
    RepeatMode repeat_mode = RepeatMode::TILE;

    Color sampleColor(int u, int v) const{

        switch (repeat_mode){
            case RepeatMode::TILE:
                return accessData(
                    u % w,
                    v % h
                );
                break;

            case RepeatMode::EXTEND:
                return accessData(
                    std::clamp(u, 0, w-1),
                    std::clamp(v, 0, h-1)
                );
                break;
            
            case RepeatMode::MIRROR:

                u = u % (w * 2 );
                v = v % (h * 2 );

                if (u<0) u += w;
                if (v<0) v += h;

                if (u >= w) u = 2*w - u;
                if (v >= h) v = 2*h - v;
                return accessData(
                    u,
                    v
                );
                break;
        }
    }

    virtual ~Texture() = default;

    Vec3 sampleVector(int u, int v) const{

        return sampleColor(u, v).toVec3();
    }

    Vec3 sampleVector(Vec2 uv) const{ // Normalized coordinates!

        return sampleColor(lround(uv[0] * (w-1)), lround(uv[1] * (h-1))).toVec3();
    }
};

class ImageTexture : public Texture{
public:


    std::vector< Color > data;

    ImageTexture(int w, int h){
        this->w = w;
        this->h = h;
    }



    static std::unique_ptr< ImageTexture > fromPPM(const std::string& path) {
        
        ppmLoader::ImageRGB im;
        ppmLoader::load_ppm(im, path);
        auto p = std::make_unique<ImageTexture>(im.w, im.h);
        p->data = std::move(im.data);
        return p;
    }

    Color accessData(int u, int v) const override{
        return data[v * w + u];

    }


};