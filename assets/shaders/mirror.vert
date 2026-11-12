#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 ProjCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform vec4 clipPlane;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);
    Normal = normalize(normalMatrix * aNormal);
    TexCoords = aTexCoords;

    gl_ClipDistance[0] = dot(worldPos, clipPlane);

    vec4 clipPos = projection * view * worldPos;
    ProjCoords = clipPos;
    gl_Position = clipPos;
}
