#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    // Sample from the cubemap texture
    vec4 texColor = texture(skybox, TexCoords);
    
    // If texture is black, use a debug color
    if (texColor.r < 0.01 && texColor.g < 0.01 && texColor.b < 0.01) {
        // Debug: make different faces different colors
        if (abs(TexCoords.x) > abs(TexCoords.y) && abs(TexCoords.x) > abs(TexCoords.z)) {
            if (TexCoords.x > 0) FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red for +X
            else FragColor = vec4(0.0, 1.0, 0.0, 1.0); // Green for -X
        } else if (abs(TexCoords.y) > abs(TexCoords.z)) {
            if (TexCoords.y > 0) FragColor = vec4(0.0, 0.0, 1.0, 1.0); // Blue for +Y
            else FragColor = vec4(1.0, 1.0, 0.0, 1.0); // Yellow for -Y
        } else {
            if (TexCoords.z > 0) FragColor = vec4(1.0, 0.0, 1.0, 1.0); // Magenta for +Z
            else FragColor = vec4(0.0, 1.0, 1.0, 1.0); // Cyan for -Z
        }
    } else {
        FragColor = texColor;
    }
}