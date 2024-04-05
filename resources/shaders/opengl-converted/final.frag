#version 450

uniform sampler2D SPIRV_Cross_CombinedoffscreenTexoffscreenSampler;

layout(location = 0) out vec4 outFragColor;
layout(location = 0) in vec2 inUV;

void main()
{
    outFragColor = texture(SPIRV_Cross_CombinedoffscreenTexoffscreenSampler, inUV);
}

