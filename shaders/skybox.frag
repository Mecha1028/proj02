#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    // Sample from the cubemap texture
    FragColor = texture(skybox, TexCoords);
}