#version 450 core

layout(points) in;
layout(line_strip, max_vertices=2) out;

in vec3 color[];
in vec3 normal[];

out vec3 fcolor;

void main(){
    gl_Position = gl_in[0].gl_Position;
    fcolor = vec3(0.2f, 0.2f, 1.0f);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + 0.5*vec4(normal[0], 0.0f);
    fcolor = vec3(0.2f, 0.2f, 1.0f);
    EmitVertex();
    EndPrimitive();
}
