#ifndef GL_MODEL_LOADER_H
#define GL_MODEL_LOADER_H

#include <graphics/resource_loaders/model_loader.h>
#include <map>
#include "pool_manager.h"

class AssimpLoader;
struct ModelData;
struct MeshData;

int modelGetTexID(Resource::Model model,
		  Resource::Texture texture,
                  BasePoolManager *pools);

class InternalModelLoader : public ModelLoader {
public:

    InternalModelLoader(Resource::Pool pool, BasePoolManager* pools);
    
    virtual ~InternalModelLoader();
    
    ModelInfo::Model loadModelData(std::string path) override;

    virtual void loadGPU() = 0;

    virtual void clearGPU() = 0;

    void clearStaged();
    
protected:    
    Resource::Model loadData(PipelineInput format,
			     ModelInfo::Model &model,
			     std::vector<void*> &meshVertData,
			     std::string textureFolder,
			     std::vector<Resource::ModelAnimation> *pAnimations) override;
    
    void loadQuad();

    Resource::Pool pool;    
    BasePoolManager *pools;
    Resource::Model quad;
    std::vector<ModelData*> staged;

private:
    
    AssimpLoader* loader;
};


/// ------- Model and Mesh Staging Data -------

struct MeshData {
    MeshData(ModelInfo::Mesh &mesh, void* vertexData,
	     std::string texturePath,
	     TextureLoader* tex);
    ~MeshData();
    void* vertices;
    size_t vertexCount;
    std::vector<uint32_t> indices;
    //temp until pipeline changes
    Resource::Texture texture;
    glm::vec4 diffuseColour;
};

struct ModelData {
    ModelData(ModelInfo::Model &model, PipelineInput format,
	      std::vector<void*> meshVertData,
	      //temp
	      std::string texturePath,
	      TextureLoader* tex);
    ~ModelData();
    PipelineInput format;
    std::vector<MeshData*> meshes;
    std::vector<Resource::ModelAnimation> animations;
};


/// ------ Model and Mesh GPU Data -------

struct GPUMesh {
    GPUMesh(){}
    GPUMesh(MeshData* mesh);

    Resource::Texture texture;
    glm::vec4 diffuseColour;
};

struct GPUModel {
    GPUModel(ModelData* model);
    
    Resource::ModelAnimation getAnimation(int index);
    
    Resource::ModelAnimation getAnimation(std::string animation);


    std::vector<Resource::ModelAnimation> animations;
    std::map<std::string, int> animationMap;
    PipelineInput vertType;
};

#endif
