#version 450 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNorm;
layout(location=2) in vec3 aColor;
out vec3 color;
out vec3 normal;

uniform mat4 projection;
uniform mat4 view_model;

void main(){
    gl_Position = projection * view_model * vec4(aPos, 1.0f);
    normal = vec3(transpose(inverse(view_model)) * vec4(aNorm,0.0f));
    color = clamp(aPos, 0.2f, 1.0f);
}
