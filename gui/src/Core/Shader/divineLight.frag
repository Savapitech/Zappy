#version 330 core
in vec2 TexCoords;
in float Height;

out vec4 FragColor;

uniform float u_Time;
uniform vec3 u_Color;
uniform float u_MaxHeight;

void main() {
    float normalizedHeight = Height / u_MaxHeight;
    float verticalFade = sin(normalizedHeight * 3.14159); 
    
    float speed = 2.0;
    float stripeFrequency = 5.0;
    float descentEffect = fract((TexCoords.y * stripeFrequency) - (u_Time * speed));
    
    descentEffect = smoothstep(0.0, 0.5, descentEffect) * smoothstep(1.0, 0.5, descentEffect);
    float wobble = sin((TexCoords.x * 10.0) + u_Time) * 0.5 + 0.5;

    float intensity = 0.3 + (descentEffect * 0.7 * wobble);
    float finalAlpha = intensity * verticalFade;

    FragColor = vec4(u_Color, finalAlpha);
}