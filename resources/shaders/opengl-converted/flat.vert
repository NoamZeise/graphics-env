#version 450
#ifdef GL_ARB_shader_draw_parameters
#extension GL_ARB_shader_draw_parameters : enable
#endif

layout(binding = 0, std140) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 0, std430) readonly buffer PerFrameBuffer
{
    mat4 model[];
} pid;

layout(location = 0) out vec3 outTexCoord;
layout(location = 1) in vec2 inTexCoord;
#ifdef GL_ARB_shader_draw_parameters
#define SPIRV_Cross_BaseInstance gl_BaseInstanceARB
#else
uniform int SPIRV_Cross_BaseInstance;
#endif
layout(location = 0) in vec3 inPos;

void main()
{
    outTexCoord = vec3(inTexCoord, float((gl_InstanceID + SPIRV_Cross_BaseInstance)));
    vec4 fragPos = (ubo.view * pid.model[(gl_InstanceID + SPIRV_Cross_BaseInstance)]) * vec4(inPos, 1.0);
    gl_Position = ubo.proj * fragPos;
}

