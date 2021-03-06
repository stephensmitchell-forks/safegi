#ifndef _PATH_H_
#define _PATH_H_

#include "tracer.h"
#include "tracerutils.h"

struct PathIntegrator : public Integrator {
    int minLength, maxLength;

    PathIntegrator() : minLength(2), maxLength(4) { }

    virtual spectrum<radiance_d> l(const point<world_s>& o, const direction<world_s>& w,  const mreal<time_d>& t, shared_ptr<Scene> scene) {
        return l(o,w,t,scene,0,true);
    }

    virtual spectrum<radiance_d> l(const point<world_s>& o, const direction<world_s>& w, const mreal<time_d>& t, shared_ptr<Scene> scene, int depth, bool includeLe) {
        // intersect the scene
        auto is = scene->intersect(ray<world_s>(o,w),t);

        // if missed, return 0
        if(!is.hit) return spectrum<radiance_d>();

        // rename variables for readability
        auto wo = -w;
        auto n = is.dp.z;
        auto dp = is.dp;
        auto m = is.m;

        // radiance: emitted + direct + indirect
        return ((includeLe)?le(wo,t,dp,m,scene,depth):spectrum<radiance_d>()) + ld(wo,t,dp,m,scene,depth) + li(wo,t,dp,m,scene,depth);
    }

    virtual spectrum<radiance_d> le(const direction<world_s>& wo, const mreal<time_d>& t, const frame<world_s,local_s>& dp, shared_ptr<Material> m, shared_ptr<Scene> scene, int depth) {
        return m->emission(wo, dp);
    }

    virtual spectrum<radiance_d> ld(const direction<world_s>& wo, const mreal<time_d>& t, const frame<world_s,local_s>& dp, shared_ptr<Material> m, shared_ptr<Scene> scene, int depth) {
        // select a random light
        auto ls = uniformsample(scene->lights);
        if(!isvalid(ls)) return spectrum<radiance_d>();

        // pick a sample on the light
        auto ss = ls.light->sampleShadow(dp.o, tuple2(urandom(), urandom()), t);
        if(!isvalid(ss)) return spectrum<radiance_d>();

        // shade
        auto lit = ss.le * m->f(wo, ss.wi, dp) * abscos(dp.z,ss.wi) / (ss.pdf * ls.pdf);
        if(iszero(lit)) return spectrum<radiance_d>();

        // shadow check
        if(scene->hit(ray<world_s>(dp.o, ss.wi, ss.dist), t)) return spectrum<radiance_d>();

        // done
        return lit;
    }

    virtual spectrum<radiance_d> li(const direction<world_s>& wo, const mreal<time_d>& t, const frame<world_s,local_s>& dp, shared_ptr<Material> m, shared_ptr<Scene> scene, int depth) {
        // check for max length
        if(depth >= maxLength) return spectrum<radiance_d>();

        // perform russian roulette if needed
        auto rrs = (depth > minLength) ? roussianRoulette(urandom(),__asreal(averageCmp(m->rho(wo,dp,true,true)))) : rrSample();
        if(!isvalid(rrs)) return spectrum<radiance_d>();
        // BUG: check roussian roulette

        // sample the material
        auto bs = m->sample(wo,dp,tuple2(urandom(),urandom()),urandom(),true,true);
        if(!isvalid(bs)) return spectrum<radiance_d>();

        // accumulate
        return bs.f * abscos(dp.z,bs.wi) * l(dp.o,bs.wi,t,scene,depth+1,bs.delta) / (bs.pdf * rrs.pdf);
    }
};

#endif
