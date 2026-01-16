#include <iostream>
#include <vector>

#include "Skybox.h"

#include <stb/stb_image.h>

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

Skybox::Skybox() : VAO(0), VBO(0), shaderId(0), cubemapTexture(0) {}  // ADD cubemapTexture initialization

Skybox::~Skybox() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (cubemapTexture) glDeleteTextures(1, &cubemapTexture);  // ADD THIS: Clean up cubemap texture
}

void Skybox::init(GLuint shaderProgram) {
    shaderId = shaderProgram;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // Load cubemap textures
    std::vector<std::string> faces = {
        "textures/skybox/posx.jpg",  // Right
        "textures/skybox/negx.jpg",  // Left
        "textures/skybox/posy.jpg",  // Top
        "textures/skybox/negy.jpg",  // Bottom
        "textures/skybox/posz.jpg",  // Front
        "textures/skybox/negz.jpg"   // Back
    };

    cubemapTexture = loadCubemap(faces);
}
GLuint Skybox::loadCubemap(std::vector<std::string> faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;

    std::string basePath = "../../../";

    for (unsigned int i = 0; i < faces.size(); i++) {
        std::string fullPath = basePath + faces[i];

        unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = GL_RGB;
            if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Failed to load skybox texture: " << fullPath << std::endl;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void Skybox::draw(glm::mat4 view, glm::mat4 projection) {
    if (cubemapTexture == 0) {
        std::cout << "WARNING: cubemapTexture is 0 (not initialized)" << std::endl;
    }

    glDepthFunc(GL_LEQUAL);  // Change depth function so skybox passes depth test
    glUseProgram(shaderId);

    // Remove translation from view matrix for skybox (skybox stays at origin)
    glm::mat4 viewWithoutTranslation = glm::mat4(glm::mat3(view));

    GLuint viewLoc = glGetUniformLocation(shaderId, "view");
    GLuint projLoc = glGetUniformLocation(shaderId, "projection");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &viewWithoutTranslation[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

    // Bind cubemap texture to texture unit 3
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    GLuint skyboxLoc = glGetUniformLocation(shaderId, "skybox");
    if (skyboxLoc == -1) {
        std::cout << "ERROR: 'skybox' uniform not found in shader!" << std::endl;
    }
    else {
        glUniform1i(skyboxLoc, 3);  // Texture unit 3
    }

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);  // Set depth function back to default
}