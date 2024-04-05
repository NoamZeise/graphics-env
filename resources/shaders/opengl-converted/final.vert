#version 450

layout(binding = 0, std140) uniform UniformBufferObject
{
    mat4 screenTransform;
} ubo;

layout(location = 0) out vec2 outUV;

void main()
{
    outUV = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));
    gl_Position = ubo.screenTransform * vec4((outUV * 2.0) - vec2(1.0), 0.0, 1.0);
}

