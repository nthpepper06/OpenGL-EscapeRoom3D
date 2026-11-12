#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aSize;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 viewPos;

out vec3 WorldPos;
out float ParticleSize;

void main()
{
    WorldPos = aPos;
    ParticleSize = aSize;

    vec4 viewSpace = view * vec4(aPos, 1.0);
    gl_Position = projection * viewSpace;

    // Attenuate point size by distance to camera
    float dist = length(viewSpace.xyz);
    gl_PointSize = clamp(aSize * (400.0 / max(dist, 0.5)), 2.0, 32.0);
}
