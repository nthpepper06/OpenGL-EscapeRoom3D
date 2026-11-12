#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform float vignetteStrength;
uniform float vignetteRadius;
uniform float chromaticAberrationStrength;
uniform float fearFactor; // 0.0 (calm) to 1.0 (panic)

uniform float exposure;
uniform bool toneMappingEnabled;
uniform bool gammaCorrectionEnabled;
uniform bool bypassTonemap;

// Krzysztof Narkowicz ACES Filmic Tone Mapping fit
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec2 uv = TexCoords;

    // Diagnostic bypass for PBR debug views (Albedo, Normal, Roughness, Metallic, AO)
    if (bypassTonemap) {
        FragColor = vec4(texture(screenTexture, uv).rgb, 1.0);
        return;
    }
    vec2 center = vec2(0.5, 0.5);
    vec2 toCenter = uv - center;
    float dist = length(toCenter);

    // Dynamic chromatic aberration: increases with distance from center and player fear
    float totalAberration = chromaticAberrationStrength + fearFactor * 0.015;
    vec2 dir = (dist > 0.0001) ? normalize(toCenter) : vec2(0.0);

    // RGB channel separation (Chromatic Aberration) on raw HDR radiance
    float r = texture(screenTexture, uv + dir * (totalAberration * dist * 1.5)).r;
    float g = texture(screenTexture, uv).g;
    float b = texture(screenTexture, uv - dir * (totalAberration * dist * 1.5)).b;
    vec3 sceneColor = vec3(r, g, b);

    // Vignette calculation
    float effectiveRadius = clamp(vignetteRadius - fearFactor * 0.15, 0.2, 1.2);
    float effectiveStrength = clamp(vignetteStrength + fearFactor * 0.35, 0.0, 1.0);
    float vignette = smoothstep(effectiveRadius, effectiveRadius * 0.35, dist);
    sceneColor *= mix(1.0, vignette, effectiveStrength);

    // Horror Heartbeat / Panic blood fringe at higher fear
    if (fearFactor > 0.05) {
        float pulse = 0.5 + 0.5 * sin(time * (4.0 + fearFactor * 8.0));
        float edgeGlow = smoothstep(0.35, 0.75, dist) * fearFactor * pulse;
        vec3 bloodRed = vec3(0.55, 0.05, 0.05);
        sceneColor = mix(sceneColor, bloodRed, edgeGlow * 0.55);
    }

    // Subtle VHS / analog noise grain
    float grain = (fract(sin(dot(uv + vec2(time * 0.05), vec2(12.9898, 78.233))) * 43758.5453) - 0.5) * 0.018;
    sceneColor += vec3(grain);

    // Photographic exposure scaling
    sceneColor *= (exposure > 0.0) ? exposure : 1.0;

    // ACES Filmic Tone Mapping (maps HDR [0, inf) -> LDR [0, 1])
    if (toneMappingEnabled) {
        sceneColor = ACESFilm(sceneColor);
    }

    // Linear to sRGB color space conversion (Gamma 2.2 correction)
    if (gammaCorrectionEnabled) {
        sceneColor = pow(sceneColor, vec3(1.0 / 2.2));
    }

    FragColor = vec4(sceneColor, 1.0);
}
