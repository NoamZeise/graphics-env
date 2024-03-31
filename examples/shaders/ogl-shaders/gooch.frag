#version 430

layout (location = 0) in vec2 inTexCoords;
layout (location = 1) in vec3 inFragPos;
layout (location = 2) in vec3 inNormal;

out vec4 outColour;

uniform sampler2D image;
uniform vec4 spriteColour;
uniform bool enableTex;

struct LightingParams
{
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec4 direction;
  vec4 camPos;
};

uniform LightingParams lighting;

void main() {
  vec4 objectColour = spriteColour;
  vec3 obj = objectColour.xyz;
  vec3 cool = vec3(0, 0, 0.55) + 0.25*obj;
  vec3 warm = vec3(0.3, 0.3, 0) + 0.5*obj;
  vec3 highlight = vec3(1, 1, 1);
  
  vec3 n = normalize(inNormal);
  vec3 l = -lighting.direction.xyz;
  vec3 v = normalize(lighting.camPos.xyz - inFragPos);
  
  float t = (dot(n,l) + 1)/2;
  vec3 r = 2*dot(n,l)*n - l;
  float s = clamp(pow(dot(r,v), lighting.specular.w), 0, 1);
  
  vec3 shaded = s*highlight + (1-s)*(t*warm + (1-t)*cool);  
  outColour = vec4(shaded.xyz, objectColour.w);
}
