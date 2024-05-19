#ifndef OUTFACING_FONT_LOADER_H
#define OUTFACING_FONT_LOADER_H

#include "../resources.h"
#include <string>

class FontLoader {
 public:
    virtual Resource::Font load(std::string file) = 0;
    virtual float length(Resource::Font font, std::string text, float size) = 0;
};

#endif
