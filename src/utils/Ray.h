#ifndef RAY_H
#define RAY_H
#include "src/utils/Line.h"
class Ray : public Line {
public:
    Vec3 invdir; // used for aabb testing
    Ray() : Line() {}
    Ray( Vec3 const & o , Vec3 const & d ) : Line(o,d), invdir(1.0 / d[0],1.0 / d[1],1.0 / d[2] ) {}
};
#endif
