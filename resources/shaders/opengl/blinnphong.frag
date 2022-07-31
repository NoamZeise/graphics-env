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

void main()
{
  vec4 objectColour;
  if(enableTex)
    objectColour = texture(image, inTexCoords) * spriteColour;
  else
    objectColour = spriteColour;

  if(objectColour.w == 0)
    discard;

  vec3 ambient = lighting.ambient.xyz * lighting.ambient.w;

  vec3 normal = normalize(inNormal);
  vec3 lightDir = normalize(-lighting.direction.xyz);

  float lambertian = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = lighting.diffuse.xyz * lighting.diffuse.w * lambertian;

  float specularIntensity = 0.0;
  if(lambertian > 0.0)
  {
    vec3 viewDir = normalize(lighting.camPos.xyz - inFragPos);

    vec3 halfDir = normalize(lightDir + viewDir);
    specularIntensity = pow(max(dot(normal, halfDir), 0.0), lighting.specular.w);
  }
  vec3 specular = lighting.specular.xyz * specularIntensity;

  outColour = vec4(ambient + diffuse + specular, 1.0) * objectColour;
}
