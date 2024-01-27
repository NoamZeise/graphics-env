#version 430

layout (location = 0) in vec2 inUV;
out vec4 colour;

uniform sampler2D screenTexture;

struct Props {
  float time;
};

uniform Props ps;

void main() {
  vec2 uv = inUV;
  uv.x *= (1 - sin(ps.time));
  uv.y *= (1 - sin(ps.time));
  uv.x *= uv.x;
  uv.y *= uv.y;
  colour = texture(screenTexture, uv);
}
