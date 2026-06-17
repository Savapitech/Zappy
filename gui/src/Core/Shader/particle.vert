#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in float aSize;

out vec3 fColor;
uniform mat4 u_Projection;

void main() {
    gl_Position = u_Projection * vec4(aPos, 0.0, 1.0);
    fColor = aColor;
    gl_PointSize = aSize;
}
