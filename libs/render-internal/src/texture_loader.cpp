#include <render-internal/resource-loaders/texture_loader.h>
#include "stb_image.h"
#include <graphics/logger.h>

#include <stdexcept>

InternalTexLoader::InternalTexLoader(Resource::Pool pool, RenderConfig conf) {
    this->pool = pool;
    this->srgb = conf.srgb;
    this->mipmapping = conf.mip_mapping;
    this->filterNearest = conf.texture_filter_nearest;
}

InternalTexLoader::~InternalTexLoader() {
    clearStaged();
}

Resource::Texture InternalTexLoader::load(std::string path) {
    for(int i = 0; i < staged.size(); i++)
	if(staged[i]->pathedTex && staged[i]->path == path) {
	    return Resource::Texture(
		    i, glm::vec2(staged[i]->width, staged[i]->height), pool);
	}
    StagedTex* tex = new StagedTex();
    tex->path = path;
    tex->pathedTex = true;
    tex->data = stbi_load(tex->path.c_str(), &tex->width, &tex->height,
			 &tex->nrChannels, desiredChannels);
    if(!tex->data) {
	LOG_ERROR("Failed to load texture - path: " << path);
	throw std::runtime_error("texture file not found");
    }
    tex->nrChannels = desiredChannels;
    tex->filesize = tex->width * tex->height * tex->nrChannels;
    return addStagedTexture(tex);
}

Resource::Texture InternalTexLoader::load(
	unsigned char* data, int width, int height, int nrChannels) {
    StagedTex* tex = new StagedTex();
    tex->data = data;
    tex->width = width;
    tex->height = height;
    tex->pathedTex = false;
    if(nrChannels != desiredChannels) {
	//TODO: CORRECT CHANNLES INSTEAD OF THROW
	throw std::runtime_error("only four channels supported");
    }
    tex->nrChannels = desiredChannels;
    tex->filesize = tex->width * tex->height * tex->nrChannels;
    return addStagedTexture(tex);
}

Resource::Texture InternalTexLoader::addStagedTexture(StagedTex *tex) {
    staged.push_back(tex);
    LOG("Texture Load"
	" - pool: " << pool.ID <<
	" - id: "   << staged.size() - 1 <<
	" - path: " << tex->path);
    Resource::Texture texture(
	    staged.size() - 1, glm::vec2(tex->width, tex->height), pool);
    stagedTextures.push_back(texture);
    return texture;
}

void StagedTex::deleteData() {
    if(data != nullptr) {
	if(pathedTex)
	    stbi_image_free(data);
	else
	    delete[] data;
	data = nullptr;
    }
}

void InternalTexLoader::clearStaged() {
    for(auto& s: staged) {
	s->deleteData();
	delete s;
    }
    staged.clear();
    stagedTextures.clear();
}
