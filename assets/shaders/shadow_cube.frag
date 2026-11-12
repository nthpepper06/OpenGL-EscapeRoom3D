#version 330 core
in vec3 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
    float lightDistance = length(FragPos - lightPos);
    // Map to [0;1] range by dividing by farPlane
    lightDistance = lightDistance / farPlane;
    gl_FragDepth = lightDistance;
}
