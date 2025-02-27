#pragma once


#include <vector>
#include <string>
#include "src/utils/Vec3.h"
#include "src/utils/Ray.h"
#include "Triangle.h"
#include "src/render/Material.h"
#include <GL/glut.h>
#include <algorithm>
#include <cfloat>


// -------------------------------------------
// Basic Mesh class
// -------------------------------------------




struct MeshVertex {
    inline MeshVertex () {}
    inline MeshVertex (const Vec3 & _p, const Vec3 & _n) : position (_p), normal (_n) , u(0) , v(0) {}
    inline MeshVertex (const MeshVertex & vertex) : position (vertex.position), normal (vertex.normal) , u(vertex.u) , v(vertex.v) {}
    inline virtual ~MeshVertex () {}
    inline MeshVertex & operator = (const MeshVertex & vertex) {
        position = vertex.position;
        normal = vertex.normal;
        u = vertex.u;
        v = vertex.v;
        return (*this);
    }
    // membres :
    Vec3 position; // une position
    Vec3 normal; // une normale
    float u,v; // coordonnees uv
};

struct MeshTriangle {
    inline MeshTriangle () {
        v[0] = v[1] = v[2] = 0;
    }
    inline MeshTriangle (const MeshTriangle & t) {
        v[0] = t.v[0];   v[1] = t.v[1];   v[2] = t.v[2];
    }
    inline MeshTriangle (unsigned int v0, unsigned int v1, unsigned int v2) {
        v[0] = v0;   v[1] = v1;   v[2] = v2;
    }
    unsigned int & operator [] (unsigned int iv) { return v[iv]; }
    unsigned int operator [] (unsigned int iv) const { return v[iv]; }
    inline virtual ~MeshTriangle () {}
    inline MeshTriangle & operator = (const MeshTriangle & t) {
        v[0] = t.v[0];   v[1] = t.v[1];   v[2] = t.v[2];
        return (*this);
    }
    // membres :
    unsigned int v[3];
};




class Mesh {
protected:
    void build_positions_array() {
        positions_array.resize( 3 * vertices.size() );

        //update AABB aswell.
        AABB_v1 = vertices[0].position;
        AABB_v2 = vertices[0].position;

        for( unsigned int v = 0 ; v < vertices.size() ; ++v ) {

            AABB_v1[0] = std::min(AABB_v1[0], vertices[v].position[0]);
            AABB_v1[1] = std::min(AABB_v1[1], vertices[v].position[1]);
            AABB_v1[2] = std::min(AABB_v1[2], vertices[v].position[2]);

            AABB_v2[0] = std::max(AABB_v2[0], vertices[v].position[0]);
            AABB_v2[1] = std::max(AABB_v2[1], vertices[v].position[1]);
            AABB_v2[2] = std::max(AABB_v2[2], vertices[v].position[2]);

            positions_array[3*v + 0] = vertices[v].position[0];
            positions_array[3*v + 1] = vertices[v].position[1];
            positions_array[3*v + 2] = vertices[v].position[2];
        }

        //margin
        AABB_v1 -= Vec3(0.0001, 0.0001, 0.0001);
        AABB_v2 += Vec3(0.0001, 0.0001, 0.0001);
    }
    void build_normals_array() {
        normalsArray.resize( 3 * vertices.size() );
        for( unsigned int v = 0 ; v < vertices.size() ; ++v ) {
            normalsArray[3*v + 0] = vertices[v].normal[0];
            normalsArray[3*v + 1] = vertices[v].normal[1];
            normalsArray[3*v + 2] = vertices[v].normal[2];
        }
    }
    void build_UVs_array() {
        uvs_array.resize( 2 * vertices.size() );
        for( unsigned int vert = 0 ; vert < vertices.size() ; ++vert ) {
            uvs_array[2*vert + 0] = vertices[vert].u;
            uvs_array[2*vert + 1] = vertices[vert].v;
        }
    }
    void build_triangles_array() {
        triangles_array.resize( 3 * triangles.size() );

        triangle_primitives.resize(triangles.size()); // for RT

        for( unsigned int t = 0 ; t < triangles.size() ; ++t ) {
            triangles_array[3*t + 0] = triangles[t].v[0];
            triangles_array[3*t + 1] = triangles[t].v[1];
            triangles_array[3*t + 2] = triangles[t].v[2];

            triangle_primitives[t] = Triangle(vertices[ triangles[t].v[0] ].position,vertices[ triangles[t].v[1] ].position,vertices[ triangles[t].v[2] ].position);
            
        }
    }

    // https://tavianator.com/2011/ray_box.html
    bool intersection(const Ray & r) const {


        float tx1 = (AABB_v1[0] - r.origin()[0]) / r.direction()[0];
        float tx2 = (AABB_v2[0] - r.origin()[0]) / r.direction()[0];

        float tmin = std::min(tx1, tx2);
        float tmax = std::max(tx1, tx2);

        float ty1 = (AABB_v1[1]- r.origin()[1]) / r.direction()[1];
        float ty2 = (AABB_v2[1]- r.origin()[1]) / r.direction()[1];

        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));

        float tz1 = (AABB_v1[2]- r.origin()[2]) / r.direction()[2];
        float tz2 = (AABB_v2[2]- r.origin()[2]) / r.direction()[2];

        tmin = std::max(tmin, std::min(tz1, tz2));
        tmax = std::min(tmax, std::max(tz1, tz2));

        return tmax >= tmin;
    }



public:
    std::vector<MeshVertex> vertices;
    std::vector<MeshTriangle> triangles;

    std::vector<Triangle> triangle_primitives;

    std::vector< float > positions_array;
    std::vector< float > normalsArray;
    std::vector< float > uvs_array;
    std::vector< unsigned int > triangles_array;

    Vec3 AABB_v1, AABB_v2;
    int material_id;
    bool cull_backfaces = true;

    void loadOFF (std::string filename);
    void recomputeNormals ();
    void centerAndScaleToUnit ();
    void scaleUnit ();

    virtual ~Mesh() = default;
    
    virtual
    void build_arrays() {
        recomputeNormals();
        build_positions_array();
        build_normals_array();
        build_UVs_array();
        build_triangles_array();
    }


    void translate( Vec3 const & translation ){
        for( unsigned int v = 0 ; v < vertices.size() ; ++v ) {
            vertices[v].position += translation;
        }
    }

    void apply_transformation_matrix( Mat3 transform ){
        for( unsigned int v = 0 ; v < vertices.size() ; ++v ) {
            vertices[v].position = transform*vertices[v].position;
        }

        //        recomputeNormals();
        //        build_positions_array();
        //        build_normals_array();
    }

    void scale( Vec3 const & scale ){
        Mat3 scale_matrix(scale[0], 0., 0.,
                0., scale[1], 0.,
                0., 0., scale[2]); //Matrice de transformation de mise à l'échelle
        apply_transformation_matrix( scale_matrix );
    }

    void rotate_x ( float angle ){
        float x_angle = angle * M_PI / 180.;
        Mat3 x_rotation(1., 0., 0.,
                        0., cos(x_angle), -sin(x_angle),
                        0., sin(x_angle), cos(x_angle));
        apply_transformation_matrix( x_rotation );
    }

    void rotate_y ( float angle ){
        float y_angle = angle * M_PI / 180.;
        Mat3 y_rotation(cos(y_angle), 0., sin(y_angle),
                        0., 1., 0.,
                        -sin(y_angle), 0., cos(y_angle));
        apply_transformation_matrix( y_rotation );
    }

    void rotate_z ( float angle ){
        float z_angle = angle * M_PI / 180.;
        Mat3 z_rotation(cos(z_angle), -sin(z_angle), 0.,
                        sin(z_angle), cos(z_angle), 0.,
                        0., 0., 1.);
        apply_transformation_matrix( z_rotation );
    }


    void draw(const Material & material) const {
        if( triangles_array.size() == 0 ) return;

        GLfloat material_color[4] = {material.diffuse_color[0],
                                     material.diffuse_color[1],
                                     material.diffuse_color[2],
                                     1.0};

        GLfloat material_specular[4] = {material.specular_color[0],
                                        material.specular_color[1],
                                        material.specular_color[2],
                                        1.0};

        GLfloat material_ambient[4] = {material.ambient_color[0],
                                       material.ambient_color[1],
                                       material.ambient_color[2],
                                       1.0};

        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState (GL_NORMAL_ARRAY);
        glNormalPointer (GL_FLOAT, 3*sizeof (float), (GLvoid*)(normalsArray.data()));
        glVertexPointer (3, GL_FLOAT, 3*sizeof (float) , (GLvoid*)(positions_array.data()));
        glDrawElements(GL_TRIANGLES, triangles_array.size(), GL_UNSIGNED_INT, (GLvoid*)(triangles_array.data()));


        // simple debug draw for AABB 

        glPointSize(5);   
        glColor3f(0.5f, 1.0f, 5.0f);
        glBegin(GL_POINTS);

        /*
        for (int i = 0; i < 30; ++i){
            glVertex3f(
                AABB_v1[0] + AABB_v2[0] * i / 30.0,
                AABB_v1[1] + AABB_v2[1] * i / 30.0,
                AABB_v1[2] + AABB_v2[2] * i / 30.0
            );
        }
        */
        glVertex3f(
            AABB_v1[0],
            AABB_v1[1],
            AABB_v1[2]
        );

        glVertex3f(
            AABB_v2[0],
            AABB_v2[1],
            AABB_v2[2]
        );

        glEnd();

    }

    RayTriangleIntersection intersect( Ray const & ray) const {
        RayTriangleIntersection closestIntersection;
        closestIntersection.intersectionExists = false;
        closestIntersection.t = FLT_MAX;

        if (!intersection(ray)) return closestIntersection;

        // si dans AABB

        
        for (Triangle triangle: triangle_primitives){

            RayTriangleIntersection res = triangle.intersect(ray, cull_backfaces);

            if (res.intersectionExists && res.t < closestIntersection.t) closestIntersection = res;
        }
        
        return closestIntersection;
    }
};

