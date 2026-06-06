#version 330 core
out vec4 FragColor;

uniform float u_Alpha;
void main() 
{
    FragColor = vec4(0.0, 0.0, 0.0, u_Alpha);
}
