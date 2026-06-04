#version 410 core
out vec4 FragColor;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

uniform sampler2D ourTexture;
uniform sampler2D shadowMap;

void main() {
    vec4 texColor = texture(ourTexture, TexCoord);
    if (texColor.a < 0.1) discard;

    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float shadow = 0.0;
    if (projCoords.z <= 1.0) {
        float currentDepth = projCoords.z;
        float bias = 0.002;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0;
    }

    shadow *= 0.5;

    vec3 sunColor = vec3(1.2, 1.1, 0.9);
    vec3 ambient = vec3(0.55, 0.6, 0.75); 

    vec3 result = texColor.rgb * (ambient + sunColor * (1.0 - shadow));

    FragColor = vec4(result, texColor.a);
}