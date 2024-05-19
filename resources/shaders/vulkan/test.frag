#version 450

layout(set = 2, binding = 0) uniform sampler texSamp;
layout(set = 2, binding = 1) uniform texture2D tex;
layout(set = 2, binding = 2) uniform texture2D textures[20];

struct per2DFragData
{
    vec4 colour;
    vec4 texOffset;
    uint texID;
};

layout(std140, set = 3, binding = 0) readonly buffer PerInstanceBuffer {
    per2DFragData data[];
} pib;

layout(set = 4, binding = 0) uniform texture2D texs[25];
layout(set = 4, binding = 1) uniform sampler samps[2];

layout(set = 5, binding = 0) uniform UBO {
  mat4 mat;
  vec4 vec;
} ubo[110];

layout(push_constant) uniform pcs
{
  mat4 matI;
  int mID;
} pc;

layout(location = 0) in vec3 inTexCoord;

layout(location = 0) out vec4 outColour;

vec4 calcColour(vec4 texOffset, vec4 colour, uint texID)
{
    vec2 coord = inTexCoord.xy;
    coord.x *= texOffset.z;
    coord.y *= texOffset.w;
    coord.x += texOffset.x;
    coord.y += texOffset.y;

    vec4 col = texture(sampler2D(textures[texID], texSamp), coord) * colour;

    if(col.w == 0)
        discard;
    return col;
}

void main()
{
    uint index = uint(inTexCoord.z);
    outColour = calcColour(pib.data[index].texOffset, pib.data[index].colour, pib.data[index].texID);

}
