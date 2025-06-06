#ifndef RESOURCE_TEXTURE_LOADER_H
#define RESOURCE_TEXTURE_LOADER_H

#include <graphics/resource_loaders/texture_loader.h>
#include <graphics/render_config.h>
#include <vector>

#include <graphics/logger.h>

struct StagedTex {
    unsigned char* data;
    int width, height, nrChannels, filesize;
    std::string path;
    bool pathedTex;
    // for renderers to subclass their own staged texture
    // to know if this is of internal type or not
    bool internalTex = false;
    virtual void deleteData();
};

class InternalTexLoader : public TextureLoader {
public:
    InternalTexLoader(Resource::Pool pool, RenderConfig conf);
    virtual ~InternalTexLoader();
    Resource::Texture load(std::string path) override;
    Resource::Texture load(unsigned char* data,
				  int width,
				  int height,
				  int nrChannels) override;

    Resource::Texture addStagedTexture(StagedTex* tex);

    virtual void loadGPU() {
	loadedTextures = stagedTextures;
	stagedTextures.clear();
    }
    void clearStaged();
    virtual void clearGPU() {
	loadedTextures.clear();
    }

    virtual unsigned int getViewIndex(Resource::Texture tex) { return tex.ID; }

    std::vector<Resource::Texture> getTextures() override { return loadedTextures; }

 protected:
    bool srgb, mipmapping, filterNearest;
    Resource::Pool pool;
    int desiredChannels = 4;

    std::vector<StagedTex*> staged;
    
    std::vector<Resource::Texture> stagedTextures;
    std::vector<Resource::Texture> loadedTextures;
};


#endif
