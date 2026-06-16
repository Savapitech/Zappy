#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 u_MVP;

void main() 
{
    TexCoords = vec2(aTexCoords.x, 1.0 - aTexCoords.y);
    gl_Position = u_MVP * vec4(aPos, 0.0, 1.0);
}
