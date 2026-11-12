#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;

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

// Global Highlight & Debug Controls
uniform vec3 highlightColor;
uniform float highlightFactor;
uniform int debugViewMode; // 0: Final, 1: Albedo, 2: Normal, 3: Roughness, 4: Metallic, 5: AO, 6: IBL-only

const float PI = 3.14159265358979323846;

// ----------------------------------------------------------------------------
// Screen-Space Cotangent Frame (Tangent fallback for meshes lacking tangents)
// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
// Trowbridge-Reitz GGX Normal Distribution Function
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0000001);
}

// ----------------------------------------------------------------------------
// Schlick-GGX Geometry Masking/Shadowing
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.0000001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// ----------------------------------------------------------------------------
// Fresnel-Schlick Approximation
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // 1. Resolve Albedo (Hardware sRGB decodes directly to Linear space)
    vec3 albedo = material.albedoColor;
    if (material.useAlbedoMap) {
        vec4 texColor = texture(material.albedoMap, TexCoords);
        albedo *= texColor.rgb;
    }

    // 2. Resolve Normal vector with TBN and Screen-Space Derivative fallback
    vec3 N = normalize(Normal);
    if (material.useNormalMap) {
        vec3 tangentNormal = texture(material.normalMap, TexCoords).xyz * 2.0 - 1.0;
        // Verify vertex TBN validity; fallback to dFdx/dFdy cotangent frame if missing
        if (length(TBN[0]) > 0.1 && length(TBN[1]) > 0.1) {
            N = normalize(TBN * tangentNormal);
        } else {
            mat3 dF_TBN = cotangent_frame(N, FragPos, TexCoords);
            N = normalize(dF_TBN * tangentNormal);
        }
    }

    // 3. Resolve Metallic
    float metallic = material.metallic;
    if (material.useMetallicMap) {
        metallic *= texture(material.metallicMap, TexCoords).r;
    }
    metallic = clamp(metallic, 0.0, 1.0);

    // 4. Resolve Roughness (clamped to prevent GGX microfacet singularity)
    float roughness = material.roughness;
    if (material.useRoughnessMap) {
        roughness *= texture(material.roughnessMap, TexCoords).r;
    }
    roughness = clamp(roughness, 0.04, 1.0);

    // 5. Resolve Ambient Occlusion
    float ao = material.ao;
    if (material.useAoMap) {
        ao *= texture(material.aoMap, TexCoords).r;
    }
    ao = clamp(ao, 0.0, 1.0);

    // 6. Resolve Emissive
    vec3 emissive = material.emissiveColor * material.emissiveIntensity;
    if (material.useEmissiveMap) {
        vec4 emissiveTex = texture(material.emissiveMap, TexCoords);
        emissive *= emissiveTex.rgb;
    }

    // ------------------------------------------------------------------------
    // DEBUG VIEW MODES (Early Exit for Shader Diagnostics)
    // ------------------------------------------------------------------------
    if (debugViewMode == 1) {
        FragColor = vec4(albedo, 1.0);
        return;
    } else if (debugViewMode == 2) {
        FragColor = vec4(N * 0.5 + 0.5, 1.0);
        return;
    } else if (debugViewMode == 3) {
        FragColor = vec4(vec3(roughness), 1.0);
        return;
    } else if (debugViewMode == 4) {
        FragColor = vec4(vec3(metallic), 1.0);
        return;
    } else if (debugViewMode == 5) {
        FragColor = vec4(vec3(ao), 1.0);
        return;
    }

    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-V, N);

    // Base dielectric reflectance F0 is 0.04; metals use surface albedo
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Reflectance accumulation equation
    vec3 Lo = vec3(0.0);

    // ------------------------------------------------------------------------
    // LIGHT 1: POINT LIGHT (Ceiling / Altar Lamp)
    // ------------------------------------------------------------------------
    if (pointLight.enabled && pointLight.intensity > 0.001) {
        vec3 L = normalize(pointLight.position - FragPos);
        vec3 H = normalize(V + L);

        float distance = length(pointLight.position - FragPos);
        float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance));
        vec3 radiance = pointLight.color * pointLight.intensity * attenuation;

        // Cook-Torrance Specular BRDF terms
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= (1.0 - metallic);

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // ------------------------------------------------------------------------
    // LIGHT 2: SPOTLIGHT (Player Flashlight with Smoothstep Penumbra)
    // ------------------------------------------------------------------------
    if (spotLight.enabled && spotLight.intensity > 0.001) {
        vec3 L = normalize(spotLight.position - FragPos);
        vec3 H = normalize(V + L);

        float distance = length(spotLight.position - FragPos);
        float attenuation = 1.0 / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * (distance * distance));

        // Smoothstep angular falloff cone (cubic Hermite interpolation)
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
            vec3 kD = vec3(1.0) - kS;
            kD *= (1.0 - metallic);

            float NdotL = max(dot(N, L), 0.0);
            Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        }
    }

    // ------------------------------------------------------------------------
    // IMAGE-BASED LIGHTING (IBL) SPLIT-SUM (EPIC GAMES / KARIS)
    // ------------------------------------------------------------------------
    vec3 ambient;
    {
        vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= (1.0 - metallic);

        // 1. Diffuse Irradiance Term
        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuseIBL = irradiance * albedo;

        // 2. Prefiltered Specular Reflection Term
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 specularIBL = prefilteredColor * (F * brdf.x + brdf.y);

        // 3. Combine with AO and Per-Room IBL Intensity
        ambient = (kD * diffuseIBL + specularIBL) * (ao * ambientAO) * iblIntensity;
    }

    // Debug Mode 6: IBL Only
    if (debugViewMode == 6) {
        FragColor = vec4(ambient, 1.0);
        return;
    }

    vec3 color = ambient + Lo + emissive;

    // ------------------------------------------------------------------------
    // INSPECTOR / HOVER HIGHLIGHT
    // ------------------------------------------------------------------------
    if (highlightFactor > 0.0) {
        color = mix(color, highlightColor, highlightFactor);
    }

    FragColor = vec4(color, 1.0);
}
