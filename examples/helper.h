#ifndef EXAMPLES_HELPER_H
#define EXAMPLES_HELPER_H

#include <graphics/manager.h>
#include <cstring>

inline RenderFramework parseArgs(int argc, char**argv, std::string* winTitle) {
    RenderFramework framework = RenderFramework::Vulkan;
    bool changed = false;
    for(int i = 0; i < argc; i++) {
    	if(strcmp(argv[i], "opengl") == 0 || strcmp(argv[i], "ogl") == 0) {
    	    framework = RenderFramework::OpenGL;
	    *winTitle += " - opengl backend";
	    changed = true;
	}
    }
    if(!changed)
	*winTitle += " - vulkan backend";
    return framework;
}

#endif
