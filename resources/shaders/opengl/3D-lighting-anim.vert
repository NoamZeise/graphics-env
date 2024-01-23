#version 430 core
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in ivec4 inBoneIDs;
layout(location = 4) in vec4 inWeights;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outFragPos;
layout(location = 2) out vec3 outNormal;

uniform mat4 model;
uniform mat4 normal;
uniform mat4 view;
uniform mat4 projection;
const int MAX_BONES = 80;
uniform mat4 bones[MAX_BONES];

void main()
{
    outTexCoord = inTexCoord;

    mat4 skin = mat4(0.0f);
    for(int i = 0; i < 4; i++) {
      skin += inWeights[i] * bones[inBoneIDs[i]];
    }

    vec4 fragPos = model * skin * vec4(inPos, 1.0f);
    outNormal = mat3(normal) * mat3(skin) * inNormal;

    gl_Position = projection * view * fragPos;
    outFragPos = vec3(fragPos) / fragPos.w;
}
