#version 430

in vec2 TexCoords;
flat in int instanceID;
out vec4 colour;

const int MAX_2D_INSTANCE = 10000;

layout (std430, binding = 5) buffer perInstanceTexOffsetData
{
  vec4 texOffset[MAX_2D_INSTANCE];
};

uniform sampler2D image;
uniform vec4 spriteColour;
uniform bool enableTex;

void main()
{
  vec2 coord = TexCoords;
  coord.x *= texOffset[instanceID].z;
  coord.y *= texOffset[instanceID].w;
  coord.x += texOffset[instanceID].x;
  coord.y += texOffset[instanceID].y;
  if(enableTex)
      colour = texture(image, coord) * spriteColour;
  else
      colour = spriteColour;

  if(colour.w == 0)
    discard;
}
