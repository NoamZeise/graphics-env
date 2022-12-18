#include "render.h"

#include <iostream>

Render::Render(RenderFramework preferredRenderer) {
    switch (preferredRenderer) {
        case RenderFramework::VULKAN:
            if(vkenv::Render::LoadVulkan()) {
                renderer = RenderFramework::VULKAN;
            }
            std::cout << "Failed to load Vulkan, trying OpenGL\n";

        case RenderFramework::OPENGL:
            if(glenv::Render::LoadOpenGL()) {
                renderer = RenderFramework::OPENGL;
            }
            else {
                std::cout <<"Failed to load OpenGL";
                noApiLoaded = true;
            }
            break;
    }
}
