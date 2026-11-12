#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 ProjCoords;

uniform sampler2D planarReflectionMap;
uniform vec2 screenSize;
uniform float bloomThreshold; // e.g. 1.15
uniform vec3 viewPos;

void main()
{
    // Screen-space texture coordinates from projected coordinates
    vec2 screenUV = (ProjCoords.xy / ProjCoords.w) * 0.5 + 0.5;
    screenUV = clamp(screenUV, 0.001, 0.999);

    // Sample the reflected scene rendered into planarReflectionMap
    vec3 refl = texture(planarReflectionMap, screenUV).rgb;

    // 1. Physical Glass Substrate Tint (antique soda-lime glass subtle green-blue tint)
    refl *= vec3(0.94, 0.98, 0.96);

    // 2. Glancing-angle Fresnel rim (subtle front-surface glass reflectivity)
    vec3 V = normalize(viewPos - FragPos);
    vec3 N = normalize(Normal);
    float NdotV = max(dot(N, V), 0.0);
    float glassFresnel = pow(1.0 - NdotV, 4.0);
    vec3 rimGlass = vec3(0.70, 0.80, 0.90);
    refl = mix(refl, rimGlass, glassFresnel * 0.15);

    // 3. Procedural Antique Grime / Dust Edge Patina
    vec2 uvDist = abs(TexCoords - vec2(0.5)) * 2.0;
    float maxDist = max(uvDist.x, uvDist.y);
    float edgeGrime = pow(maxDist, 4.0);
    float dustNoise = fract(sin(dot(floor(TexCoords * 48.0), vec2(12.9898, 78.233))) * 43758.5453);
    float grimeMask = clamp(edgeGrime * 0.35 + dustNoise * edgeGrime * 0.15, 0.0, 0.50);
    vec3 dustColor = vec3(0.22, 0.20, 0.18);
    refl = mix(refl, dustColor, grimeMask);

    // 4. Soft-Shoulder Compression relative to Bloom Threshold (bloomThreshold = 1.15)
    // Ensures mirror radiance smoothly asymptotes to 0.90 and NEVER reaches bloomThreshold (1.15)
    // Flashlight shining directly at the mirror will NEVER produce a white blowout or bloom!
    float effThreshold = (bloomThreshold > 0.0) ? bloomThreshold : 1.15;
    float safeCap = effThreshold - 0.25; // 0.90, comfortably below HDR bloom threshold
    float maxLum = max(refl.r, max(refl.g, refl.b));
    if (maxLum > 0.70) {
        float shoulder = 0.70 + (safeCap - 0.70) * (1.0 - exp(-(maxLum - 0.70) / 0.35));
        refl = refl * (shoulder / maxLum);
    }
    refl = min(refl, vec3(safeCap));

    FragColor = vec4(refl, 1.0);
}
