#version 450

struct per2DFragData
{
    vec4 colour;
    vec4 texOffset;
    uint texID;
};

layout(binding = 0, std430) readonly buffer PerInstanceBuffer
{
    per2DFragData data[];
} pib;

uniform sampler2D SPIRV_Cross_CombinedtexturestexSamp[20];

layout(location = 0) in vec3 inTexCoord;
layout(location = 0) out vec4 outColour;

vec4 calcColour(vec4 texOffset, vec4 colour, uint texID)
{
    vec2 coord = inTexCoord.xy;
    coord.x *= texOffset.z;
    coord.y *= texOffset.w;
    coord.x += texOffset.x;
    coord.y += texOffset.y;
    vec4 col = texture(SPIRV_Cross_CombinedtexturestexSamp[texID], coord) * colour;
    if (col.w == 0.0)
    {
        discard;
    }
    return col;
}

void main()
{
    uint index = uint(inTexCoord.z);
    vec4 param = pib.data[index].texOffset;
    vec4 param_1 = pib.data[index].colour;
    uint param_2 = pib.data[index].texID;
    vec4 _115 = calcColour(param, param_1, param_2);
    outColour = _115;
}

