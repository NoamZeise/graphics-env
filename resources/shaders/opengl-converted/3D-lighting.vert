#version 450
#ifdef GL_ARB_shader_draw_parameters
#extension GL_ARB_shader_draw_parameters : enable
#endif

struct Obj3DPerFrame
{
    mat4 model;
    mat4 normalMat_M;
};

layout(binding = 0, std430) readonly buffer PerInstanceData
{
    Obj3DPerFrame data[];
} pid;

layout(binding = 0, std140) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out vec2 outTexCoord;
layout(location = 2) in vec2 inTexCoord;
#ifdef GL_ARB_shader_draw_parameters
#define SPIRV_Cross_BaseInstance gl_BaseInstanceARB
#else
uniform int SPIRV_Cross_BaseInstance;
#endif
layout(location = 0) in vec3 inPos;
layout(location = 2) out vec3 outNormal_world;
layout(location = 1) in vec3 inNormal;
layout(location = 1) out vec3 outFragPos_world;

void main()
{
    outTexCoord = inTexCoord;
    vec4 fragPos = pid.data[(gl_InstanceID + SPIRV_Cross_BaseInstance)].model * vec4(inPos, 1.0);
    outNormal_world = vec3((pid.data[(gl_InstanceID + SPIRV_Cross_BaseInstance)].normalMat_M * vec4(inNormal, 0.0)).xyz);
    gl_Position = (ubo.proj * ubo.view) * fragPos;
    outFragPos_world = vec3(fragPos.xyz) / vec3(fragPos.w);
}

