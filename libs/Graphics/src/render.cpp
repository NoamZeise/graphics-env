#include "render.h"

#include <iostream>

Render::Render(RenderFramework preferredRenderer) {
    switch (preferredRenderer) {
        case RenderFramework::VULKAN:
            if(vkenv::Render::LoadVulkan()) {
                renderer = RenderFramework::VULKAN;
                break;
            }
            std::cout << "Failed to load Vulkan, trying OpenGL\n";

        case RenderFramework::OPENGL:
            if(glenv::GLRender::LoadOpenGL()) {
                renderer = RenderFramework::OPENGL;
                break;
            }
            else {
                std::cout <<"Failed to load OpenGL\n";
                noApiLoaded = true;
            }
            break;
    }
}
