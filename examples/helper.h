#ifndef EXAMPLES_HELPER_H
#define EXAMPLES_HELPER_H

#include <graphics/manager.h>
#include <cstring>

inline void parseArgs(int argc, char**argv, ManagerState *state) {
    bool backendChanged = false;
    for(int i = 0; i < argc; i++) {
    	if(strcmp(argv[i], "opengl") == 0 || strcmp(argv[i], "ogl") == 0) {
    	    state->defaultRenderer = RenderFramework::OpenGL;
	    backendChanged = true;
	}
	if(strcmp(argv[i], "choose-device") == 0) {
	    state->render.manuallyChoseGpu = true;
	}
    }
    if(state->defaultRenderer == RenderFramework::Vulkan)
	state->windowTitle += " - vulkan backend";
    if(state->defaultRenderer == RenderFramework::OpenGL)
	state->windowTitle += " - opengl backend";
}

#endif
