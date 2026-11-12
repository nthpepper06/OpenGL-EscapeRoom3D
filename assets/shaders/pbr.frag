#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;
in vec4 FragPosSpotSpace;

struct Material {
    vec3 albedoColor;
    float metallic;
    float roughness;
    float ao;
    vec3 emissiveColor;
    float emissiveIntensity;

    bool useAlbedoMap;
    sampler2D albedoMap;

    bool useNormalMap;
    sampler2D normalMap;

    bool useMetallicMap;
    sampler2D metallicMap;

    bool useRoughnessMap;
    sampler2D roughnessMap;

    bool useAoMap;
    sampler2D aoMap;

    bool useEmissiveMap;
    sampler2D emissiveMap;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    bool enabled;
};

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

uniform Material material;
uniform PointLight pointLight;
uniform SpotLight spotLight;
uniform vec3 viewPos;

// IBL Split-Sum Inputs
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform float iblIntensity;
uniform float ambientAO;

// Phase 2: Shadows
uniform sampler2D spotShadowMap;
uniform samplerCube pointShadowCubemap;
uniform bool spotShadowEnabled;
uniform bool pointShadowEnabled;
uniform float pointLightFarPlane;

uniform float spotBiasMin;
uniform float spotBiasMax;
uniform float spotNormalBias;
uniform float spotPcfRadius;
uniform int spotPcfTaps;

uniform float pointBiasMin;
uniform float pointBiasMax;
uniform float pointNormalBias;

// Phase 2: SSAO
uniform sampler2D ssaoTexture;
uniform bool ssaoEnabled;
uniform vec2 screenSize;

// Phase 2: Fog & In-scatter
uniform vec3 fogColor;
uniform float fogDensity;
uniform float fogHeightFalloff;
uniform float inscatterIntensity;

// Global Highlight & Debug Controls
uniform vec3 highlightColor;
uniform float highlightFactor;
uniform int debugViewMode; // 0: Final, 1: Albedo, 2: Normal, 3: Roughness, 4: Metallic, 5: AO, 6: IBL-only, 7: Shadow-only, 8: SSAO-only

const float PI = 3.14159265358979323846;

// 16-sample Poisson disk
const vec2 poissonDisk[16] = vec2[](
    vec2(-0.94201624, -0.39906216),
    vec2( 0.94558609, -0.76890725),
    vec2(-0.09418410, -0.92938870),
    vec2( 0.34495938,  0.29387760),
    vec2(-0.91588581,  0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543,  0.27676845),
    vec2( 0.97484398,  0.75648379),
    vec2( 0.44323325, -0.97511554),
    vec2( 0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2( 0.79197514,  0.19090188),
    vec2(-0.24188840,  0.99706507),
    vec2(-0.81409955,  0.91437590),
    vec2( 0.19984126,  0.78641367),
    vec2( 0.14383161, -0.14100790)
);

float randomAngle(vec2 seed) {
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453) * 6.2831853;
}

// Tangent fallback
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
    return mat3(T * invmax, B * invmax, N);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / max(denom, 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / max(denom, 0.0000001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float calculateSpotShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    if (!spotShadowEnabled) return 0.0;

    // Normal offset bias to push position away along normal
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;
    }

    float cosTheta = max(dot(normal, lightDir), 0.0);
    float bias = max(spotBiasMax * (1.0 - cosTheta), spotBiasMin);

    float angle = randomAngle(gl_FragCoord.xy);
    float s = sin(angle), c = cos(angle);
    mat2 rot = mat2(c, -s, s, c);
    float filterRadius = (1.0 / 1024.0) * spotPcfRadius;

    float shadow = 0.0;
    int taps = clamp(spotPcfTaps, 4, 16);

    for (int i = 0; i < taps; ++i) {
        vec2 offset = rot * poissonDisk[i] * filterRadius;
        float closestDepth = texture(spotShadowMap, projCoords.xy + offset).r;
        if (projCoords.z - bias > closestDepth) {
            shadow += 1.0;
        }
    }
    return shadow / float(taps);
}

float calculatePointShadow(vec3 fragPos, vec3 lightPos, vec3 normal) {
    if (!pointShadowEnabled) return 0.0;

    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    if (currentDepth >= pointLightFarPlane) return 0.0;

    vec3 L = normalize(-fragToLight);
    float cosTheta = max(dot(normal, L), 0.0);
    float bias = max(pointBiasMax * (1.0 - cosTheta), pointBiasMin);

    // Normal offset for point light
    vec3 biasedFragToLight = (fragPos + normal * pointNormalBias) - lightPos;

    float closestDepth = texture(pointShadowCubemap, biasedFragToLight).r * pointLightFarPlane;
    float shadow = (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
    return shadow;
}

void main()
{
    // 1. Albedo
    vec3 albedo = material.albedoColor;
    if (material.useAlbedoMap) {
        vec4 texColor = texture(material.albedoMap, TexCoords);
        albedo *= pow(texColor.rgb, vec3(2.2));
    }

    // 2. Normal Mapping with fallback
    vec3 N;
    if (material.useNormalMap) {
        vec3 normalSample = texture(material.normalMap, TexCoords).rgb * 2.0 - 1.0;
        if (length(TBN[0]) > 0.01 && length(TBN[1]) > 0.01) {
            N = normalize(TBN * normalSample);
        } else {
            mat3 cotTBN = cotangent_frame(normalize(Normal), FragPos, TexCoords);
            N = normalize(cotTBN * normalSample);
        }
    } else {
        N = normalize(Normal);
    }

    // 3. Metallic, Roughness, AO
    float metallic = material.metallic;
    if (material.useMetallicMap) metallic *= texture(material.metallicMap, TexCoords).r;

    float roughness = material.roughness;
    if (material.useRoughnessMap) roughness *= texture(material.roughnessMap, TexCoords).r;
    roughness = clamp(roughness, 0.04, 1.0);

    float ao = material.ao;
    if (material.useAoMap) ao *= texture(material.aoMap, TexCoords).r;

    // SSAO Modulation
    float ssaoFactor = 1.0;
    if (ssaoEnabled && screenSize.x > 0.0 && screenSize.y > 0.0) {
        ssaoFactor = texture(ssaoTexture, gl_FragCoord.xy / screenSize).r;
    }

    // 4. Emissive
    vec3 emissive = material.emissiveColor * material.emissiveIntensity;
    if (material.useEmissiveMap) {
        emissive *= texture(material.emissiveMap, TexCoords).rgb;
    }

    // Debug View Modes
    if (debugViewMode == 1) { FragColor = vec4(albedo, 1.0); return; }
    if (debugViewMode == 2) { FragColor = vec4(N * 0.5 + 0.5, 1.0); return; }
    if (debugViewMode == 3) { FragColor = vec4(vec3(roughness), 1.0); return; }
    if (debugViewMode == 4) { FragColor = vec4(vec3(metallic), 1.0); return; }
    if (debugViewMode == 5) { FragColor = vec4(vec3(ao), 1.0); return; }
    if (debugViewMode == 8) { FragColor = vec4(vec3(ssaoFactor), 1.0); return; }

    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // ------------------------------------------------------------------------
    // LIGHT 1: POINT LIGHT (Ceiling Lamp) + Shadow
    // ------------------------------------------------------------------------
    vec3 Lo_point = vec3(0.0);
    float pointShadow = 0.0;
    if (pointLight.enabled && pointLight.intensity > 0.001) {
        vec3 L = normalize(pointLight.position - FragPos);
        vec3 H = normalize(V + L);

        float distance = length(pointLight.position - FragPos);
        float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance));
        vec3 radiance = pointLight.color * pointLight.intensity * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
        float NdotL = max(dot(N, L), 0.0);

        Lo_point = (kD * albedo / PI + specular) * radiance * NdotL;
        pointShadow = calculatePointShadow(FragPos, pointLight.position, N);
    }

    // ------------------------------------------------------------------------
    // LIGHT 2: SPOTLIGHT (Flashlight) + Soft Shadow
    // ------------------------------------------------------------------------
    vec3 Lo_spot = vec3(0.0);
    float spotShadow = 0.0;
    if (spotLight.enabled && spotLight.intensity > 0.001) {
        vec3 L = normalize(spotLight.position - FragPos);
        vec3 H = normalize(V + L);

        float distance = length(spotLight.position - FragPos);
        float attenuation = 1.0 / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * (distance * distance));

        float theta = dot(L, normalize(-spotLight.direction));
        float spotIntensity = smoothstep(spotLight.outerCutOff, spotLight.cutOff, theta);

        if (spotIntensity > 0.0) {
            vec3 radiance = spotLight.color * (spotLight.intensity * spotIntensity * attenuation);

            float NDF = DistributionGGX(N, H, roughness);
            float G   = GeometrySmith(N, V, L, roughness);
            vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 numerator    = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            vec3 specular     = numerator / denominator;

            vec3 kS = F;
            vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
            float NdotL = max(dot(N, L), 0.0);

            Lo_spot = (kD * albedo / PI + specular) * radiance * NdotL;
            spotShadow = calculateSpotShadow(FragPosSpotSpace, N, L);
        }
    }

    // ------------------------------------------------------------------------
    // IMAGE-BASED LIGHTING (IBL) SPLIT-SUM
    // ------------------------------------------------------------------------
    vec3 ambient;
    {
        vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metallic);

        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuseIBL = irradiance * albedo;

        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 specularIBL = prefilteredColor * (F * brdf.x + brdf.y);

        // Modulate with material baked AO, ambientAO, and dynamic SSAO
        ambient = (kD * diffuseIBL + specularIBL) * (ao * ambientAO * ssaoFactor) * iblIntensity;
    }

    if (debugViewMode == 6) { FragColor = vec4(ambient, 1.0); return; }
    if (debugViewMode == 7) { FragColor = vec4(vec3(1.0 - max(pointShadow, spotShadow)), 1.0); return; }

    // Direct lighting decoupled from ambient: shadow multiplies only direct terms!
    vec3 directLighting = (1.0 - pointShadow) * Lo_point + (1.0 - spotShadow) * Lo_spot;
    vec3 color = ambient + directLighting + emissive;

    // ------------------------------------------------------------------------
    // EXPONENTIAL HEIGHT FOG + FLASHLIGHT IN-SCATTER
    // ------------------------------------------------------------------------
    if (fogDensity > 0.0) {
        float distToCam = length(viewPos - FragPos);
        float heightFactor = exp(-fogHeightFalloff * max(0.0, FragPos.y));
        float fogFactor = 1.0 - exp(-distToCam * fogDensity * heightFactor);
        fogFactor = clamp(fogFactor, 0.0, 1.0);

        // Forward-scattering in-scatter inside flashlight beam
        if (spotLight.enabled && spotLight.intensity > 0.0) {
            vec3 toFrag = normalize(FragPos - viewPos);
            float coneDot = dot(toFrag, normalize(spotLight.direction));
            if (coneDot > spotLight.outerCutOff) {
                float inscatter = smoothstep(spotLight.outerCutOff, spotLight.cutOff, coneDot);
                color += spotLight.color * inscatter * inscatterIntensity * (1.0 - exp(-distToCam * 0.04));
            }
        }
        color = mix(color, fogColor, fogFactor);
    }

    // Inspector / Hover highlight
    if (highlightFactor > 0.0) {
        color = mix(color, highlightColor, highlightFactor);
    }

    FragColor = vec4(color, 1.0);
}
