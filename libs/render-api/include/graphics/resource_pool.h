#ifndef OUTFACING_GRAPHICS_RESOURCE_POOL
#define OUTFACING_GRAPHICS_RESOURCE_POOL

#include "resource_loaders/model_loader.h"
#include "resource_loaders/texture_loader.h"
#include "resource_loaders/font_loader.h"

class ResourcePool {
 public:
    virtual ModelLoader* model() = 0;
    virtual TextureLoader* tex() = 0;
    virtual FontLoader* font() = 0;
    Resource::Pool id() { return pool; }
protected:
    Resource::Pool pool;
};


#endif /* OUTFACING_GRAPHICS_RESOURCE_POOL */

