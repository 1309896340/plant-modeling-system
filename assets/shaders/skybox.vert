#version 450 core

layout(location=0) in vec3 aPos;

out vec3 TexCoords;

layout(std140, binding=0) uniform PV{
    mat4 projection;
    mat4 view;
};

void main(){
    TexCoords = aPos;
    mat4 tmp_view = mat4(mat3(view));
    gl_Position = projection * tmp_view * vec4(aPos, 1.0f);
}

