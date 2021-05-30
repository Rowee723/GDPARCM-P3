#version 430

in vec3 TexCoords;

out vec4 fragColor;

uniform samplerCube skybox;

void main(void)
{    
    fragColor = texture(skybox, TexCoords);
}