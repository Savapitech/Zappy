#version 410 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out vec4 FragPosLightSpace;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 u_MVP;
uniform mat4 u_Model;
uniform mat4 lightSpaceMatrix;

uniform float u_uvScaleX;
uniform float u_uvScaleY;
uniform float u_uvOffsetX;
uniform float u_uvOffsetY;

void main() {
    gl_Position = u_MVP * vec4(aPos.x, aPos.y, 0.0, 1.0);
    vec4 worldPos = u_Model * vec4(aPos.x, aPos.y, 0.0, 1.0);
    
    FragPosLightSpace = lightSpaceMatrix * worldPos;

    TexCoord = vec2(
        (aTexCoord.x * u_uvScaleX) + u_uvOffsetX, 
        (aTexCoord.y * u_uvScaleY) + u_uvOffsetY
    );

    FragPos = worldPos.xyz;
    mat3 normalMatrix = transpose(inverse(mat3(u_Model)));
    Normal = normalize(normalMatrix * vec3(0.0, 0.0, 1.0));
}