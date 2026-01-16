#ifndef __UI_H__
#define __UI_H__

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class UI {
private:
    bool showHelp;
    GLuint shaderId;
    GLuint VAO, VBO;
    int screenWidth, screenHeight;

    void renderButton(float x, float y, float width, float height,
        glm::vec3 color, const std::string& label);
    void renderText(float x, float y, const std::string& text, glm::vec3 color);

public:
    UI();
    ~UI();

    void init(GLuint shaderProgram, int width, int height);
    void toggleHelp();
    bool isHelpShown() const { return showHelp; }

    void updateScreenSize(int width, int height);
    void render();

    void printControlsToConsole();
};

#endif