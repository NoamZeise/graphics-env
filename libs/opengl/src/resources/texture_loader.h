#ifndef GLTEXTURE_LOADER_H
#define GLTEXTURE_LOADER_H

#include <glad/glad.h>
#include <resource_loader/texture_loader.h>
  
class TextureLoaderGL : public InternalTexLoader {
public:
    TextureLoaderGL(Resource::Pool pool, RenderConfig conf);
    ~TextureLoaderGL();
    unsigned int getViewIndex(Resource::Texture tex) override;
    void loadGPU() override;
    void clearGPU() override;
private:
    std::vector<GLuint> inGpu;
};    

#endif
