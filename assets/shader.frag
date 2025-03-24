#version 450

layout(location = 0) in vec3 fragment_color;
layout(location = 0) out vec4 out_color;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    out_color = vec4(fragment_color, 1.0);
}