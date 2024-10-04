#version 450 core

in vec3 TexCoords;

out vec4 FragColor;

layout(std140, binding=0) uniform PV{
    mat4 projection;
    mat4 view;
};

uniform samplerCube skybox;

void main(){
    FragColor = texture(skybox, TexCoords);
}

