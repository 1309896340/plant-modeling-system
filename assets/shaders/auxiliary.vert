#version 450 core

layout(location=0) in vec3 aPos;
layout(location=2) in vec3 aColor;

layout(std140, binding=0) uniform PV{
    mat4 projection;
    mat4 view;
};

// out vec3 vertPos;
out vec3 vertColor;

void main(){
    gl_Position = projection * view * vec4(aPos, 1.0f);
    vertColor = aColor;
}
