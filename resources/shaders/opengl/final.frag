#version 430

layout (location = 0) in vec2 inUV;
out vec4 colour;

uniform sampler2D screenTexture;

void main() {
  colour = texture(screenTexture, inUV);
}
