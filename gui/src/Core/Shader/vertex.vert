#version 410 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out vec4 FragPosLightSpace;

uniform mat4 u_MVP;
uniform mat4 u_Model;
uniform mat4 lightSpaceMatrix;

void main() {
    gl_Position = u_MVP * vec4(aPos.x, aPos.y, 0.0, 1.0);
    vec4 worldPos = u_Model * vec4(aPos.x, aPos.y, 0.0, 1.0);
    
    FragPosLightSpace = lightSpaceMatrix * worldPos;
    TexCoord = aTexCoord;
}