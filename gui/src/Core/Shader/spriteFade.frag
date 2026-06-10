#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D u_Texture;
uniform float u_Alpha;

void main() {
    vec4 texColor = texture(u_Texture, TexCoords);
    FragColor = vec4(texColor.rgb, texColor.a * u_Alpha);
}
