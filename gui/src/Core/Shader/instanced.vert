#version 410 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in mat4 aInstanceMatrix;

out vec2 TexCoord;
out vec4 FragPosLightSpace;

uniform mat4 u_ViewProj;
uniform mat4 lightSpaceMatrix;

void main() {
    vec4 worldPos = aInstanceMatrix * vec4(aPos.x, aPos.y, 0.0, 1.0);
    gl_Position = u_ViewProj * worldPos;

    FragPosLightSpace = lightSpaceMatrix * worldPos; 
    TexCoord = aTexCoord;
}