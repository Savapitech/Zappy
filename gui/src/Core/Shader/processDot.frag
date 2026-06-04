#version 410 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;

uniform float u_focusDistance;
uniform float u_focusRange;

float near = 0.1;
float far = 1000.0;
float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; 
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;
    float depth = texture(depthTexture, TexCoords).r;
    float z = LinearizeDepth(depth);

    float focusDistance = 43.0;
    float focusRange = 15.0;

    float blurAmount = clamp(abs(z - focusDistance) / focusRange, 0.0, 1.0);

    if (blurAmount < 0.05) {
        FragColor = vec4(color, 1.0);
        return;
    }

    vec2 texOffset = 1.0 / textureSize(screenTexture, 0);
    vec3 blurColor = vec3(0.0);
    float total = 0.0;

    for(int x = -2; x <= 2; ++x) {
        for(int y = -2; y <= 2; ++y) {
            vec2 offset = vec2(x, y) * texOffset * (blurAmount * 4.0);
            blurColor += texture(screenTexture, TexCoords + offset).rgb;
            total += 1.0;
        }
    }
    blurColor /= total;

    FragColor = vec4(mix(color, blurColor, blurAmount), 1.0);
}