#ifndef RESOURCE_FONT_LOADER
#define RESOURCE_FONT_LOADER

#include <graphics/resource_loaders/font_loader.h>
#include <graphics/resource_loaders/texture_loader.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

struct FontData;

class InternalFontLoader : public FontLoader {
public:
    InternalFontLoader(Resource::Pool pool, TextureLoader *texLoader);
    virtual ~InternalFontLoader();
    Resource::Font load(std::string file) override;
    float length(Resource::Font font, std::string text, float size) override;

    std::vector<Resource::QuadDraw> DrawString(Resource::Font font,
					       std::string text, glm::vec2 pos,
					       float size, float depth,
					       glm::vec4 colour, float rotate);
    void clearStaged();
    void loadGPU();
    void clearGPU();
    
private:
    void clearFonts(std::vector<FontData*> &fonts);
    
    Resource::Pool pool;
    TextureLoader *texLoader;
    std::vector<FontData*> staged;
    std::vector<FontData*> fonts;
};

#endif
