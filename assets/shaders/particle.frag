#version 330 core
out vec4 FragColor;

in vec3 WorldPos;
in float ParticleSize;

uniform vec3 viewPos;
uniform vec3 spotPos;
uniform vec3 spotDir;
uniform float spotCutOff;
uniform float spotOuterCutOff;
uniform vec3 spotColor;
uniform bool spotEnabled;

void main()
{
    // Soft circular mask
    vec2 coord = gl_PointCoord - vec2(0.5);
    float distSq = dot(coord, coord);
    if (distSq > 0.25) discard;

    float alpha = smoothstep(0.25, 0.05, distSq) * 0.45;

    // Ambient dust mote brightness
    vec3 baseColor = vec3(0.75, 0.78, 0.85);

    // Flashlight cone illumination boost
    if (spotEnabled) {
        vec3 toParticle = normalize(WorldPos - spotPos);
        float theta = dot(toParticle, normalize(spotDir));
        float spotFactor = smoothstep(spotOuterCutOff, spotCutOff, theta);
        if (spotFactor > 0.0) {
            float distToLight = length(WorldPos - spotPos);
            float atten = 1.0 / (1.0 + 0.15 * distToLight + 0.05 * distToLight * distToLight);
            baseColor += spotColor * (spotFactor * atten * 3.5);
            alpha = min(1.0, alpha * (1.0 + spotFactor * 2.0));
        }
    }

    FragColor = vec4(baseColor, alpha);
}
