#version 450 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNorm;
layout(location=2) in vec3 aColor;
layout(location=3) in vec2 aTexCoord;

out vec3 vertPos;
out vec3 vertNorm;
out vec3 vertColor;
out vec2 TexCoord;

layout(std140, binding=0) uniform PV{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

void main(){
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    vertPos = vec3(model * vec4(aPos, 1.0f));
    vertColor = clamp(aPos, 0.2f, 1.0f);
    vertNorm =  vec3(transpose(inverse(model)) * vec4(aNorm, 0.0f));
    TexCoord = aTexCoord;
}
