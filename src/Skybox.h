#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Skybox {
private:
    GLuint VAO, VBO;
    GLuint shaderId;
    GLuint cubemapTexture;

public:
    Skybox();
    ~Skybox();

    void init(GLuint shaderProgram);
    void draw(glm::mat4 view, glm::mat4 projection);

    GLuint loadCubemap(std::vector<std::string> faces);
};

#endif