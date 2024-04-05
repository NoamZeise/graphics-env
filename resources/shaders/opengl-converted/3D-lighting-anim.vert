#version 450
#ifdef GL_ARB_shader_draw_parameters
#extension GL_ARB_shader_draw_parameters : enable
#endif

struct Obj3DPerFrame
{
    mat4 model;
    mat4 normalMat;
};

layout(binding = 0, std140) uniform boneView
{
    mat4 mat[80];
} bones;

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
layout(location = 4) in vec4 inWeights;
layout(location = 3) in ivec4 inBoneIDs;
#ifdef GL_ARB_shader_draw_parameters
#define SPIRV_Cross_BaseInstance gl_BaseInstanceARB
#else
uniform int SPIRV_Cross_BaseInstance;
#endif
layout(location = 0) in vec3 inPos;
layout(location = 2) out vec3 outNormal;
layout(location = 1) in vec3 inNormal;
layout(location = 1) out vec3 outFragPos;

void main()
{
    outTexCoord = inTexCoord;
    mat4 skin = mat4(vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0));
    for (int i = 0; i < 4; i++)
    {
        mat4 _55 = bones.mat[inBoneIDs[i]] * inWeights[i];
        skin = mat4(skin[0] + _55[0], skin[1] + _55[1], skin[2] + _55[2], skin[3] + _55[3]);
    }
    vec4 fragPos = (pid.data[(gl_InstanceID + SPIRV_Cross_BaseInstance)].model * skin) * vec4(inPos, 1.0);
    outNormal = (mat3(pid.data[(gl_InstanceID + SPIRV_Cross_BaseInstance)].normalMat[0].xyz, pid.data[(gl_InstanceID + SPIRV_Cross_BaseInstance)].normalMat[1].xyz, pid.data[(gl_InstanceID + SPIRV_Cross_BaseInstance)].normalMat[2].xyz) * mat3(skin[0].xyz, skin[1].xyz, skin[2].xyz)) * inNormal;
    gl_Position = (ubo.proj * ubo.view) * fragPos;
    outFragPos = vec3(fragPos.xyz) / vec3(fragPos.w);
}

