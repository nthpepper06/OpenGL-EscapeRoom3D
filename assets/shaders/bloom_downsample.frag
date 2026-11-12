#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D srcTexture;
uniform vec2 srcResolution;
uniform int mipLevel;
uniform float threshold;
uniform float softKnee;

vec3 rgbToLuminance(vec3 col) {
    return vec3(dot(col, vec3(0.2126, 0.7152, 0.0722)));
}

vec3 quadraticThreshold(vec3 color, float thresh, float knee) {
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float soft = luma - thresh + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = (soft * soft) / (4.0 * knee + 0.00001);
    float contribution = max(soft, luma - thresh);
    contribution /= max(luma, 0.00001);
    return color * max(0.0, contribution);
}

void main()
{
    vec2 texelSize = 1.0 / srcResolution;
    float x = texelSize.x;
    float y = texelSize.y;

    // 13-tap bilinear downsample filter (Jimenez / Call of Duty / Karis)
    vec3 a = texture(srcTexture, vec2(TexCoords.x - 2.0*x, TexCoords.y + 2.0*y)).rgb;
    vec3 b = texture(srcTexture, vec2(TexCoords.x,          TexCoords.y + 2.0*y)).rgb;
    vec3 c = texture(srcTexture, vec2(TexCoords.x + 2.0*x, TexCoords.y + 2.0*y)).rgb;

    vec3 d = texture(srcTexture, vec2(TexCoords.x - 2.0*x, TexCoords.y)).rgb;
    vec3 e = texture(srcTexture, vec2(TexCoords.x,          TexCoords.y)).rgb;
    vec3 f = texture(srcTexture, vec2(TexCoords.x + 2.0*x, TexCoords.y)).rgb;

    vec3 g = texture(srcTexture, vec2(TexCoords.x - 2.0*x, TexCoords.y - 2.0*y)).rgb;
    vec3 h = texture(srcTexture, vec2(TexCoords.x,          TexCoords.y - 2.0*y)).rgb;
    vec3 i = texture(srcTexture, vec2(TexCoords.x + 2.0*x, TexCoords.y - 2.0*y)).rgb;

    vec3 j = texture(srcTexture, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 k = texture(srcTexture, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;
    vec3 l = texture(srcTexture, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 m = texture(srcTexture, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;

    vec3 downsample = e * 0.125;
    downsample += (a + c + g + i) * 0.03125;
    downsample += (b + d + f + h) * 0.0625;
    downsample += (j + k + l + m) * 0.125;

    // For mip 0, apply thresholding to filter out non-emissive geometry
    if (mipLevel == 0) {
        downsample = quadraticThreshold(downsample, threshold, softKnee);
    }

    FragColor = vec4(downsample, 1.0);
}
