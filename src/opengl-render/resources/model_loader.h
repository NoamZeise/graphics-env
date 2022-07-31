#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#ifndef NO_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#include "vertex_data.h"
#include "texture_loader.h"
#include "resources.h"


namespace Resource
{

class ModelLoader
{
public:
	ModelLoader();
	~ModelLoader();
	Model LoadModel(std::string path, TextureLoader* texLoader);
  void DrawModel(Model model, TextureLoader* texLoader, uint32_t spriteColourShaderLoc);
  void DrawModelInstanced(Model model, TextureLoader* texLoader, int count, uint32_t spriteColourShaderLoc, uint32_t enableTexShaderLoc);

private:
	struct Mesh
	{
		VertexData* vertexData;
		Texture texture;
        glm::vec4 diffuseColour;
	};
	struct LoadedModel
	{
		LoadedModel() {}
		~LoadedModel()
		{
			for(unsigned int j = 0; j < meshes.size(); j++)
			{
				delete meshes[j].vertexData;
			}
		}
		std::vector<Mesh> meshes;
		std::string directory;
	};

#ifndef NO_ASSIMP
  void processNode(LoadedModel* model, aiNode* node, const aiScene* scene, TextureLoader* texLoader, aiMatrix4x4 parentTransform);
	void processMesh(Mesh* mesh, aiMesh* aimesh, const aiScene* scene, TextureLoader* texLoader, aiMatrix4x4 transform);
	void loadMaterials(Mesh* mesh, aiMaterial* material, TextureLoader* texLoader);
#endif

	std::vector<LoadedModel*> loadedModels;
	std::vector<Texture> loadedTextures;

};

}

#endif
