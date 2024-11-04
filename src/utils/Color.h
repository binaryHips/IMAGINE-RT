#pragma once

#include <algorithm>
#include "src/utils/Vec3.h"
class Color{
public:
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    unsigned char a = 255;

    Color() = default;
    Color(unsigned char red, unsigned char green, unsigned char blue): r(red), g(green), b(blue) {}
    Color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha): r(red), g(green), b(blue), a(alpha) {}

    Color(const Vec3 & v){

        r = (unsigned char)(std::clamp(v[0] * 255, 0.0f, 255.0f));
        g = (unsigned char)(std::clamp(v[1] * 255, 0.0f, 255.0f));
        b = (unsigned char)(std::clamp(v[2] * 255, 0.0f, 255.0f));
    }

    inline Vec3 toVec3() const {
        return Vec3(float(r)/255.0, float(g)/255.0, float(b)/255.0);
    }

    Color& operator += (const Color& rhs){
        r = std::clamp(r+rhs.r, 0, 255);
        g = std::clamp(g+rhs.g, 0, 255);
        b = std::clamp(b+rhs.b, 0, 255);
        return *this;
    }

    Color& operator -= (const Color& rhs){
        r = std::clamp(r-rhs.r, 0, 255);
        g = std::clamp(g-rhs.g, 0, 255);
        b = std::clamp(b-rhs.b, 0, 255);
        return *this;
    }

    template<typename T> // dirty or genius? I don't know. I never know.
    Color& operator /= (const T& rhs){
        r = std::clamp(r/ static_cast<unsigned int> (rhs), 0u, 255u);
        g = std::clamp(g/ static_cast<unsigned int> (rhs), 0u, 255u);
        b = std::clamp(b/ static_cast<unsigned int> (rhs), 0u, 255u);
        return *this;
    }

    template<typename T>
    Color& operator *= (const T& rhs){
        r = std::clamp(r* static_cast<unsigned int> (rhs), 0u, 255u);
        g = std::clamp(g* static_cast<unsigned int> (rhs), 0u, 255u);
        b = std::clamp(b* static_cast<unsigned int> (rhs), 0u, 255u);
        return *this;
    }

    unsigned char & operator [](int i){
        return (i == 0)? r : (i==1)? g: b;
    }
    const unsigned char & operator [](int i) const { 
        return (i == 0)? r : (i==1)? g: b;
    }

    int luminance() const {
        return 0.299*r + 0.587*g + 0.114*b;
}
};