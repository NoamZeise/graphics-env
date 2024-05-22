#ifndef MODEL_GEN_H
#define MODEL_GEN_H

#include <graphics/resource_loaders/model_info.h>
#include <functional>

struct SurfaceParam {
    float start;
    float end;
    float step = 1;
    SurfaceParam(float start, float end, float step) {
	this->start = start;
	this->end = end;
	this->step = step;
    }
};

ModelInfo::Model genSurface(
	std::function<glm::vec3(float, float)> surfaceFn,
	bool smoothShading, float uvDensity,
	SurfaceParam a,
	SurfaceParam b);

#endif /* MODEL_GEN_H */
