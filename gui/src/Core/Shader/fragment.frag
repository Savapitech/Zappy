#version 410 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main() {
    if (FragColor.a < 0.1)
        discard;
    FragColor = texture(ourTexture, TexCoord);
}