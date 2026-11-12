#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    bool enabled;
};

uniform SpotLight spotLight;
uniform bool uIsUVMode;
uniform bool isUvLightActive;
uniform vec3 decalColor;
uniform float decalIntensity;
uniform sampler2D decalTexture;
uniform bool useDecalTexture;

void main()
{
    bool uvActive = uIsUVMode || isUvLightActive;
    if (!uvActive || !spotLight.enabled) {
        discard;
    }

    vec3 toLight = spotLight.position - FragPos;
    float dist = length(toLight);
    vec3 L = normalize(toLight);
    float theta = dot(L, normalize(-spotLight.direction));

    if (theta <= spotLight.outerCutOff || dist >= 14.0) {
        discard;
    }

    float coneFactor = smoothstep(spotLight.outerCutOff, spotLight.cutOff, theta);
    float distAtten = 1.0 / (1.0 + 0.08 * dist + 0.03 * dist * dist);
    float spotFactor = coneFactor * distAtten;

    if (spotFactor <= 0.05) {
        discard;
    }

    vec3 baseCol = decalColor;
    float alpha = 1.0;
    if (useDecalTexture) {
        vec4 tex = texture(decalTexture, TexCoords);
        baseCol *= tex.rgb;
        alpha *= tex.a;
    }

    float effIntensity = (decalIntensity > 0.0) ? decalIntensity : 4.5;
    vec3 glow = baseCol * (effIntensity * spotFactor);

    FragColor = vec4(glow, alpha * clamp(spotFactor * 2.0, 0.0, 1.0));
}
