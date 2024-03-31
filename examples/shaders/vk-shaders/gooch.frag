#version 450

layout(push_constant) uniform fragconstants
{
    vec4 colour;
    vec4 texOffset;
    int texID;
} pc;

layout(set = 3, binding = 0) uniform sampler texSamp;
layout(set = 3, binding = 1) uniform texture2D textures[20];
layout(set = 4, binding = 0) uniform LightingUBO
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 direction;
    vec4 camPos;
} lighting;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inFragPos;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColour;


void main()
{
    vec4 objectColour = pc.colour;

    vec3 obj = objectColour.xyz;
    vec3 cool = vec3(0, 0, 0.55) + 0.25*obj;
    vec3 warm = vec3(0.3, 0.3, 0) + 0.5*obj;
    vec3 highlight = vec3(1, 1, 1);

    vec3 n = normalize(inNormal);
    vec3 l = -lighting.direction.xyz;
    vec3 v = normalize(lighting.camPos.xyz - inFragPos);
    
    float t = (dot(n,l) + 1)/2;
    vec3 r = 2*dot(n,l)*n - l;
    //float s = clamp(100*dot(r,v) - 97, 0, 1);
    float s = clamp(pow(dot(r,v), lighting.specular.w), 0, 1);

    vec3 shaded = s*highlight + (1-s)*(t*warm + (1-t)*cool);

    outColour = vec4(shaded.xyz, objectColour.w);
}
