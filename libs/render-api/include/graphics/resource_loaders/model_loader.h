#ifndef OUTFACING_MODEL_LOADER
#define OUTFACING_MODEL_LOADER

#include "../resources.h"
#include "../model/animation.h"
#include "../pipeline.h"
#include <cstdlib>

class ModelLoader {
    const std::string DEFAULT_TEXTURE_PATH = "textures/";
 public:
    virtual Resource::Model load(
	    Resource::ModelType type,
	    ModelInfo::Model &modelData,
	    std::string textureFolder,
	    std::vector<Resource::ModelAnimation>* pAnimations) = 0;

    
    
    Resource::Model load(Resource::ModelType type, ModelInfo::Model &modelData) {
	return load(type, modelData, DEFAULT_TEXTURE_PATH, nullptr);
    }
    
    Resource::Model load(ModelInfo::Model &modelData) {
	return load(Resource::ModelType::m3D, modelData);
    }
    
    Resource::Model load(
	    Resource::ModelType type,
	    std::string path,
	    std::string textureFolder,
	    std::vector<Resource::ModelAnimation>* pAnimations) {
	ModelInfo::Model model = loadModelData(path);
	return load(type, model, textureFolder, pAnimations);
    }
    
    Resource::Model load(
	    Resource::ModelType type,
	    std::string path,
	    std::vector<Resource::ModelAnimation>* pAnimations) {
	return load(type, path, DEFAULT_TEXTURE_PATH, pAnimations);
    }
    
    Resource::Model load(Resource::ModelType type, std::string path, std::string textureFolder) {
	return load(type, path, textureFolder, nullptr);
    }
    
    Resource::Model load(Resource::ModelType type, std::string path) {
	return load(type, path, DEFAULT_TEXTURE_PATH, nullptr);
    }
    
    Resource::Model load(std::string path) {
	return load(Resource::ModelType::m3D, path);
    }


    template <typename T_Vert>
    Resource::Model load(ModelType<T_Vert> modelType,
			 ModelInfo::Model modelData,
			 std::string textureFolder,
			 std::vector<Resource::ModelAnimation>* pAnimations) {
	std::vector<void*> meshVertData;
	for(auto &meshData: modelData.meshes) {
	    T_Vert* vertices = (T_Vert*)std::malloc(
		    sizeof(T_Vert) * meshData.verticies.size());
	    for(int i = 0; i < meshData.verticies.size(); i++) {
		vertices[i] = modelType.vertexLoader(
			meshData.verticies[i],
			meshData.bindTransform);
	    }
	    meshVertData.push_back(vertices);
	}
	return loadData(modelType.input, modelData, meshVertData, textureFolder, pAnimations);
    }

    template <typename T_Vert>
    Resource::Model load(ModelType<T_Vert> modeltype, std::string path) {
	ModelInfo::Model model = loadModelData(path);
	return load(modeltype, model, DEFAULT_TEXTURE_PATH, nullptr);
    }
    
    
    virtual ModelInfo::Model loadModelData(std::string path) = 0;

    
    virtual Resource::ModelAnimation getAnimation(Resource::Model model, std::string animation) = 0;


    virtual Resource::ModelAnimation getAnimation(Resource::Model model, int index) = 0;

protected:
    virtual Resource::Model loadData(PipelineInput format,
				     ModelInfo::Model &model,
				     std::vector<void*> &meshVertData,
				     std::string textureFolder,
				     std::vector<Resource::ModelAnimation> *pAnimations) = 0;
};

#endif /* OUTFACING_MODEL_LOADER */
