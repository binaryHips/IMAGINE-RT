
#include "src/utils/imageLoader.h"
#include <fstream>
#include "src/utils/Vec3.h"
#include "src/utils/Color.h"
#include <vector>
#include <memory>
#include <algorithm>

enum RepeatMode {
    TILE,
    MIRROR,
    EXTEND,
}

class ImageTexture : Texture;

class Texture{

private:
    int w, h;

    virtual Color accessData(int u, int v) const;
    
public:
    RepeatMode repeat_mode = RepeatMode.TILE;

    Color sampleColor(int u, int v) const{

        switch (repeat_mode){
            case RepeatMode.TILE:
                return accessData(
                    u % w,
                    v % h
                );
                break;

            case RepeatMode.EXTEND:
                return accessData(
                    std::clamp(u, 0, w-1),
                    std::clamp(v, 0, h-1)
                );
                break;
            
            case RepeatMode.MIRROR:

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

    Vec3 sampleVector(int u, int v) const{

        return sampleColor(u, v).toVec3();
    }



    static std::sharedPointer< ImageTexture > fromOff {
        
        ppmLoader
    }


}


class ImageTexture : public Texture{
    int w, h;

    std::vector< Color >data;

    Color accessData(int u, int v) const override{


    }
}