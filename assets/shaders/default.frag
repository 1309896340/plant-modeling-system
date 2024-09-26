#version 450 core
in vec3 color;
in vec2 TexCoord;
out vec4 FragColor;

uniform bool useTexture;
uniform sampler2D tex;

void main(){
    if(useTexture){
        FragColor = texture(tex, TexCoord);
    }else{
        FragColor = vec4(color, 1.0f);
    }
}
