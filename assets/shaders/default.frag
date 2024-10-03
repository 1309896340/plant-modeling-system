#version 450 core
in vec3 vertPos;
in vec3 vertNorm;
in vec3 vertColor;
in vec2 TexCoord;
out vec4 FragColor;

// 渲染目标材质
uniform bool useTexture;    // 选择：纹理/顶点颜色
uniform bool useLight;
uniform sampler2D tex;
uniform float ambientStrength;      // 环境光系数
uniform float diffuseStrength;      // 漫反射光系数
uniform float specularStrength;     // 镜面反射系数

// 光源
uniform vec3 lightPos;
uniform vec3 lightColor;

// 相机
uniform vec3 eyePos;

// 变换矩阵
layout(std140, binding=0) uniform PV{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

void main(){
    vec3 texColor = vec3(1.0f, 1.0f, 0.0f);
    if(useTexture){
        texColor = texture(tex, TexCoord).rgb;
    }else{
        texColor = vertColor;
    }
    vec3 resColor = vec3(0.0f, 0.0f, 0.0f);
    if(useLight){
        // 环境光
        vec3 ambient = ambientStrength * lightColor;
        // 漫反射
        vec3 lightDir = normalize(lightPos - vertPos);
        vec3 diffuse = diffuseStrength * lightColor * max(dot(lightDir, vertNorm), 0.0f);
        // 镜面反射
        vec3 reflectDir = reflect(-lightDir, vertNorm);
        vec3 eyeDir = normalize(eyePos - vertPos);
        vec3 specular = specularStrength * lightColor * pow(max(dot(eyeDir, reflectDir), 0.0f), 32);
        // 颜色混合
        resColor = (ambient + diffuse + specular) * texColor;
    }else{
        resColor = texColor;
    }
    FragColor = vec4(resColor, 1.0f);
}
