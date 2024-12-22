#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "src/utils/Vec3.h"
#include "src/utils/Ray.h"
#include "Plane.h"
#include <cfloat>
struct RayTriangleIntersection{
    bool intersectionExists;
    float t;
    Vec2 uv;
    unsigned int tIndex;
    Vec3 intersection;
    Vec3 normal;
};

class Triangle {
private:
    Vec3 m_c[3] , m_normal;
    float area;
public:
    Triangle() {}
    Triangle( Vec3 const & c0 , Vec3 const & c1 , Vec3 const & c2 ) {
        m_c[0] = c0;
        m_c[1] = c1;
        m_c[2] = c2;
        updateAreaAndNormal();
    }
    void updateAreaAndNormal() {
        Vec3 nNotNormalized = Vec3::cross( m_c[1] - m_c[0] , m_c[2] - m_c[0] );
        float norm = nNotNormalized.length();
        m_normal = nNotNormalized / norm;
        area = norm / 2.f;
    }
    void setC0( Vec3 const & c0 ) { m_c[0] = c0; } // remember to update the area and normal afterwards!
    void setC1( Vec3 const & c1 ) { m_c[1] = c1; } // remember to update the area and normal afterwards!
    void setC2( Vec3 const & c2 ) { m_c[2] = c2; } // remember to update the area and normal afterwards!
    Vec3 const & normal() const { return m_normal; }
    Vec3 projectOnSupportPlane( Vec3 const & p ) const {
        Vec3 result;
        //TODO completer
        return result;
    }
    float squareDistanceToSupportPlane( Vec3 const & p ) const {
        float result;
        //TODO completer
        return result;
    }
    float distanceToSupportPlane( Vec3 const & p ) const { return sqrt( squareDistanceToSupportPlane(p) ); }
    bool isParallelTo( Line const & L ) const {
        bool result;
        //TODO completer
        return result;
    }
    Vec3 getIntersectionPointWithSupportPlane( Line const & L ) const {
        // you should check first that the line is not parallel to the plane!
        Vec3 result;
        //TODO completer
        return result;
    }
    void computeBarycentricCoordinates( Vec3 const & p , float & u0 , float & u1 , float & u2 ) const {
        //TODO Complete
    }

    RayTriangleIntersection getIntersection( Ray const & ray ) const { // implementé à partir de https://fr.wikipedia.org/wiki/Algorithme_d%27intersection_de_M%C3%B6ller-Trumbore
        RayTriangleIntersection result;
        result.t = FLT_MAX;
        Vec3 edge1, edge2, h, s, q;
        float a,f,u,v;
        edge1 = m_c[1] -  m_c[0];
        edge2 =  m_c[2] -  m_c[0];

        h = Vec3::cross(ray.direction(), edge2);
        a = Vec3::dot(edge1, h);

        if (a > -0.000001 && a < 0.000001) return result;    // Le rayon est parallèle au triangle.

        f = 1.0/a;
        s = ray.origin() - m_c[0];
        u = f * (Vec3::dot(s, h));

        if (u < 0.0 || u > 1.0) return result;

        q = Vec3::cross(s, edge1);
        v = f * Vec3::dot(ray.direction(), q);
        
        if (v < 0.0 || u + v > 1.0) return result;

        float t = f * Vec3::dot(edge2, q);
        if (t > 0)
        {

            result.intersectionExists = true;
            result.t = t;
            result.intersection = ray.at(t);
            result.normal = m_normal;

            

            result.uv = Vec2(0.0, 0.0);

            return result;
        }
        return result;
    }
};
#endif