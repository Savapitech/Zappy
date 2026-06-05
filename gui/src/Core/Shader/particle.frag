#version 330 core
in vec3 fColor;
out vec4 FragColor;

uniform float u_Alpha;

void main() 
{
    vec2 pt = gl_PointCoord - vec2(0.5);
    float dist = length(pt);
    if (dist > 0.5) 
        discard;
    float intensity = 1.0 - (dist * 2.0);
    intensity = pow(max(0.0, intensity), 1.5);
    FragColor = vec4(fColor * intensity, intensity * u_Alpha);
}
