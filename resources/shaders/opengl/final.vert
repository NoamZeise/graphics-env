#version 430 core

layout (location = 0) out vec2 outUV;

uniform mat4 screenTransform;

void main() {
  outUV = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
  gl_Position = screenTransform * vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}
