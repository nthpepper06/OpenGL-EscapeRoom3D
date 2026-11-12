#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Material {
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
    bool useTexture;
    sampler2D diffuseTexture;
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
uniform vec3 ambientColor;
uniform float ambientIntensity;
uniform vec3 viewPos;

uniform vec3 highlightColor;
uniform float highlightFactor;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffColor, vec3 specColor)
{
    if (!light.enabled) return vec3(0.0);

    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Blinn-Phong Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    vec3 diffuse = diff * light.color * diffColor;
    vec3 specular = spec * light.color * specColor;
    
    return (diffuse + specular) * light.intensity * attenuation;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffColor, vec3 specColor)
{
    if (!light.enabled) return vec3(0.0);

    vec3 lightDir = normalize(light.position - fragPos);
    
    // Check if inside spotlight cone
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float spotIntensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    if (spotIntensity <= 0.0) return vec3(0.0);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Blinn-Phong Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    vec3 diffuse = diff * light.color * diffColor;
    vec3 specular = spec * light.color * specColor;
    
    return (diffuse + specular) * light.intensity * attenuation * spotIntensity;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 baseDiffuse = material.diffuseColor;
    if (material.useTexture) {
        vec4 tex = texture(material.diffuseTexture, TexCoords);
        baseDiffuse *= tex.rgb;
    }

    vec3 baseSpecular = material.specularColor;

    // Global ambient
    vec3 ambient = ambientColor * ambientIntensity * baseDiffuse;

    // Point Light (Ceiling lamp)
    vec3 pointResult = CalcPointLight(pointLight, norm, FragPos, viewDir, baseDiffuse, baseSpecular);

    // Spotlight (Player flashlight)
    vec3 spotResult = CalcSpotLight(spotLight, norm, FragPos, viewDir, baseDiffuse, baseSpecular);

    vec3 finalColor = ambient + pointResult + spotResult;

    // Highlight for focused / selected objects
    if (highlightFactor > 0.0) {
        finalColor = mix(finalColor, highlightColor, highlightFactor);
    }

    FragColor = vec4(finalColor, 1.0);
}
