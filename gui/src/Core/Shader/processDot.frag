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

    float distanceToFocus = abs(z - u_focusDistance);

    float focusBand = u_focusRange * 0.2;
    float blurMax = u_focusRange * 0.8;

    float blurAmount = smoothstep(focusBand, blurMax, distanceToFocus);

    vec2 texOffset = 1.0 / textureSize(screenTexture, 0);
    vec3 blurColor = vec3(0.0);
    float total = 0.0;

    float blurRadius = blurAmount * 8.0;

    for(int x = -2; x <= 2; ++x) {
        for(int y = -2; y <= 2; ++y) {
            vec2 offset = vec2(x, y) * texOffset * blurRadius;
            blurColor += texture(screenTexture, TexCoords + offset).rgb;
            total += 1.0;
        }
    }
    blurColor /= total;

    FragColor = vec4(mix(color, blurColor, blurAmount), 1.0);
}