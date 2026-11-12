#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform int kernelSize;
uniform mat4 projection;
uniform vec2 noiseScale;
uniform float radius;
uniform float bias;
uniform float power;

void main()
{
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    // If fragment is too far or unwritten background, no occlusion
    if (length(fragPos) < 0.001 || length(normal) < 0.1) {
        FragColor = 1.0;
        return;
    }

    // Gram-Schmidt process to create TBN rotation frame
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    int validSamples = 0;

    for (int i = 0; i < kernelSize; ++i) {
        // Sample position in view-space
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * radius;

        // Project sample position to screen coordinates
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        // Read sample depth from gPosition
        float sampleDepth = texture(gPosition, offset.xy).z;

        // Range check to prevent distant halo artifacts
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
        validSamples++;
    }

    occlusion = 1.0 - (occlusion / float(max(validSamples, 1)));
    FragColor = pow(clamp(occlusion, 0.0, 1.0), power);
}
