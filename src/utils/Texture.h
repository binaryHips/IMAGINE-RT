
#pragma once
#include "src/utils/imageLoader.h"
#include <fstream>
#include "src/utils/Vec3.h"
#include "src/utils/Color.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <utility>
enum RepeatMode {
    TILE,
    MIRROR,
    EXTEND,
};


class Texture{

private:
    int w, h;

    virtual Color accessData(int u, int v) const;
    
public:
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

                u = u % (w * 2);
                v = v % (h * 2);

                if (u<0) u += w;
                if (v<0) v += h;

                if (u >= w-1) u = 2*w - u;
                if (v >= h-1) v = 2*h - v;

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
};

class ImageTexture : public Texture{
public:
    int w, h;

    std::vector< Color > data;

    ImageTexture(std::vector< Color >&& data, int w, int h)
    : w(w),
    h(h),
    data(std::move(data))
    {}

    static std::unique_ptr< Texture > fromOff() {
        
        ppmLoader::ImageRGB im;
        return std::make_unique<ImageTexture>(std::move(im.data), im.w, im.h);
    }

    Color accessData(int u, int v) const override{
        return data[u * w + v];

    }


};