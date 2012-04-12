#ifndef _MESH_H_
#define _MESH_H_

#include "bbox.h"
#include <memory>
using std::tr1::static_pointer_cast;

struct Mesh : public Shape {
    std::vector<point<shape_s>> pos;
    std::vector<int> faces;
    std::vector<normal<shape_s>> norm;
    shared_ptr<BBox> group;

    Mesh(const std::vector<point<shape_s>>& pos, const std::vector<int>& faces) : pos(pos), faces(faces) {
        std::vector<shared_ptr<Shape>> triangles;
        for(size_t f = 0; f < faces.size(); f += 3) {
            triangles.push_back(shared_ptr<Shape>(new Triangle(pos[faces[f]], pos[faces[f+1]], pos[faces[f+2]])));
        }
        group = shared_ptr<BBox>(new BBox(triangles));
#ifdef USE_SAFE_GL 
        gl_init();
#endif
    }

    Mesh(const std::vector<point<shape_s>>& pos, const std::vector<normal<shape_s>>& norm, 
        const std::vector<int>& faces) : pos(pos), norm(norm), faces(faces) {
            std::vector<shared_ptr<Shape>> triangles;
            for(size_t f = 0; f < faces.size(); f += 3) {
                triangles.push_back(shared_ptr<Shape>(
                    new Triangle(pos[faces[f]], pos[faces[f+1]], pos[faces[f+2]], 
                    norm[faces[f]], norm[faces[f+1]], norm[faces[f+2]])));
            }
            group = shared_ptr<BBox>(new BBox(triangles));
#ifdef USE_SAFE_GL 
        gl_init();
#endif
    }

    virtual ~Mesh() 
    {
#ifdef USE_SAFE_GL 
        gl_cleanup();
#endif
    }


    virtual isect<shape_s> intersect(const ray<shape_s>& r) {
        return group->intersect(r);
    }

    virtual bool hit(const ray<shape_s>& r) {
        return group->hit(r);
    }

    virtual shapeSample<shape_s> sample(const tuple2& ruv) { // not supported
        return shapeSample<shape_s>();
    }

    virtual mreal<area_d> area() { // not supported
        return mreal<area_d>();
    }

    virtual boundingbox<shape_s> bound() {
        return group->bound();
    }
#ifdef USE_SAFE_GL
    VaBuf<point<shape_s>>     vert_va;
    VaBuf<normal<shape_s>>    norm_va;
    virtual void gl_draw(Aid<point<shape_s>> pa, Aid<normal<shape_s>> na)
    {
        safeglEnableClientState(GL_VERTEX_ARRAY);
        safeglEnableVertexAttribArray(pa);
        safeglEnableVertexAttribArray(na);
        safeglVertexAttribPointer(pa, 0, 0, vert_va);
        safeglVertexAttribPointer(na, 0, 0, norm_va);
        safeglDrawArrays(GL_TRIANGLES, 0, faces.size());
        safeglDisableVertexAttribArray(pa);
        safeglDisableVertexAttribArray(na);
        safeglDisableClientState(GL_VERTEX_ARRAY);
    }
private:
    void gl_init()
    {
        if(!vert_va.ptr || !norm_va.ptr)
        {
            safeglCreateVertexAttribArray(vert_va, faces.size());
            safeglCreateVertexAttribArray(norm_va, faces.size());
            for(size_t f = 0; f < faces.size(); f += 3) {
                point<shape_s>& p0 = pos[faces[f]];
                point<shape_s>& p1 = pos[faces[f+1]];
                point<shape_s>& p2 = pos[faces[f+2]];
                normal<shape_s> n0, n1, n2;
                if(norm.empty())
                    n0 = n1 = n2 = asnormal(orthogonal(normalize(p0-p1), normalize(p0-p2)));
                else
                {
                    n0 = norm[faces[f]];
                    n1 = norm[faces[f+1]];
                    n2 = norm[faces[f+2]];
                }
                safeglSetVertexAttribArray(vert_va, f + 0, p0);
                safeglSetVertexAttribArray(vert_va, f + 1, p1);
                safeglSetVertexAttribArray(vert_va, f + 2, p2);
                safeglSetVertexAttribArray(norm_va, f + 0, n0);
                safeglSetVertexAttribArray(norm_va, f + 1, n1);
                safeglSetVertexAttribArray(norm_va, f + 2, n2);
            }
        }
    }

    void gl_cleanup()
    {
        if(vert_va.ptr)
            safeglDeleteVertexAttribArray(vert_va);
        if(norm_va.ptr)
            safeglDeleteVertexAttribArray(norm_va);
    }
#endif
};

#endif
