#version 410 core
out vec4 FragColor;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

in vec3 FragPos;
in vec3 Normal;

uniform sampler2D ourTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec4 texColor = texture(ourTexture, TexCoord);
    if (texColor.a < 0.1)
        discard;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 sunColor = vec3(1.2, 1.1, 0.9);
    vec3 diffuseLight = diff * sunColor;

    float rimFactor = 1.0 - max(dot(viewDir, norm), 0.0);
    rimFactor = smoothstep(0.7, 1.0, rimFactor); 
    vec3 rimColor = vec3(0.8, 0.9, 1.0);
    vec3 rimLight = rimFactor * rimColor * (diff + 0.2);

    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float shadow = 0.0;
    if (projCoords.z <= 1.0) {
        float currentDepth = projCoords.z;

        float bias = max(0.005 * (1.0 - dot(norm, lightDir)), 0.001);
        
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0;
    }
    shadow *= 0.6;

    vec3 ambient = vec3(0.35, 0.4, 0.55);

    vec3 lighting = ambient + ((1.0 - shadow) * (diffuseLight + rimLight));
    vec3 result = texColor.rgb * lighting;

    FragColor = vec4(result, texColor.a);
}