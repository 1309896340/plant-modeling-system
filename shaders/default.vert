#version 450 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNorm;
layout(location=2) in vec3 aColor;
out vec3 color;

uniform mat4 PVM;

void main(){
    gl_Position = PVM * vec4(aPos, 1.0f);
    color = clamp(aPos, 0.2f, 1.0f);
}
