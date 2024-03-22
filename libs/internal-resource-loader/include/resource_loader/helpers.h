#ifndef GRAPHICS_ENV_INTERNAL_HELPERS_H
#define GRAPHICS_ENV_INTERNAL_HELPERS_H

#include <glm/glm.hpp>
#include <graphics/render_config.h>

inline glm::vec2 getTargetRes(RenderConfig renderConf, int w, int h) {
    glm::vec2 targetResolution(renderConf.target_resolution[0],
			       renderConf.target_resolution[1]);
    if(targetResolution.x == 0.0 || targetResolution.y == 0.0)
	targetResolution = glm::vec2(w, h);
    return targetResolution;
}

#endif
