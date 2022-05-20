#version 430
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 inTexCoords;

out vec2 TexCoords;
flat out int instanceID;

const int MAX_2D_INSTANCE = 10000;

layout (std430, binding = 4) buffer perInstanceModelData
{
  mat4 model[MAX_2D_INSTANCE];
};

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = inTexCoords;
    instanceID = gl_InstanceID;
    gl_Position = projection * view * model[gl_InstanceID] * vec4(vertex, 1.0);
}
