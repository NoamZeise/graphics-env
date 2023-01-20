#ifndef GL_MODEL_RENDER_H
#define GL_MODEL_RENDER_H

#ifndef NO_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#include "vertex_data.h"
#include "texture_loader.h"

#include <resources/model_loader.h>
#include <resources/resources.h>

namespace Resource {

class GLModelRender {
public:
  GLModelRender();
  ~GLModelRender();
  Model LoadModel(std::string path, GLTextureLoader *texLoader);
  void DrawModel(Model model, GLTextureLoader *texLoader,
                 uint32_t spriteColourShaderLoc);
  void DrawModelInstanced(Model model, GLTextureLoader *texLoader, int count,
                          uint32_t spriteColourShaderLoc,
                          uint32_t enableTexShaderLoc);

private:
  struct Mesh {
    GLVertexData *vertexData;
    Texture texture;
    glm::vec4 diffuseColour;
  };
  struct LoadedModel {
    LoadedModel() {}
    ~LoadedModel() {
      for (unsigned int j = 0; j < meshes.size(); j++) {
        delete meshes[j].vertexData;
      }
    }
    std::vector<Mesh> meshes;
    std::string directory;
  };

#ifndef NO_ASSIMP
  void processNode(LoadedModel *model, aiNode *node, const aiScene *scene,
                   GLTextureLoader *texLoader, aiMatrix4x4 parentTransform);
  void processMesh(Mesh *mesh, aiMesh *aimesh, const aiScene *scene,
                   GLTextureLoader *texLoader, aiMatrix4x4 transform);
  void loadMaterials(Mesh *mesh, aiMaterial *material,
                     GLTextureLoader *texLoader);
#endif

  ModelLoader modelLoader;
  std::vector<LoadedModel *> loadedModels;
  std::vector<Texture> loadedTextures;
};

} // namespace Resource

#endif
