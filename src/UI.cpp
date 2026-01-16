#include "UI.h"
#include <iostream>
#include <GLFW/glfw3.h>

UI::UI() : showHelp(false), shaderId(0), VAO(0), VBO(0),
screenWidth(800), screenHeight(800) {
}

UI::~UI() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
}

void UI::init(GLuint shaderProgram, int width, int height) {
    shaderId = shaderProgram;
    screenWidth = width;
    screenHeight = height;

    // Simple quad vertices (normalized device coordinates)
    float vertices[] = {
        // positions  
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    GLuint EBO;
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    glDeleteBuffers(1, &EBO);  // EBO is stored in VAO state
}

void UI::toggleHelp() {
    showHelp = !showHelp;
    if (showHelp) {
        printControlsToConsole();
    }
}

void UI::updateScreenSize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
}

void UI::printControlsToConsole() {
    std::cout << "\n=== OPENGL PROJECT CONTROLS ===\n";
    std::cout << "CAMERA CONTROLS:\n";
    std::cout << "  W/A/S/D - Move camera\n";
    std::cout << "  Mouse - Look around\n";
    std::cout << "  Mouse Wheel - Zoom in/out\n";
    std::cout << "  Q/E - Move up/down\n";
    std::cout << "  R - Reset camera\n";
    std::cout << "\nMODEL CONTROLS:\n";
    std::cout << "  1-4 - Select model (Teapot, Bunny, Box, Floor)\n";
    std::cout << "  I/K - Move selected forward/backward\n";
    std::cout << "  J/L - Move selected left/right\n";
    std::cout << "  U/O - Move selected up/down\n";
    std::cout << "\nVIEW CONTROLS:\n";
    std::cout << "  H - Toggle help (this display)\n";
    std::cout << "  Ctrl+D - Toggle shadow debug view\n";
    std::cout << "  Arrow Keys - Pan/tilt camera\n";
    std::cout << "  +/- - Zoom in/out (also mouse wheel)\n";
    std::cout << "================================\n";
}

void UI::render() {
    if (!showHelp) return;

    // Save current OpenGL state
    GLint prevDepthTest, prevBlend, prevProgram;
    glGetIntegerv(GL_DEPTH_TEST, &prevDepthTest);
    glGetIntegerv(GL_BLEND, &prevBlend);
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);

    // Setup for 2D overlay
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use the UI shader
    glUseProgram(shaderId);

    // We'll use a simple approach: render multiple quads for UI elements

    // Helper function to draw a quad with given screen coordinates
    auto drawQuad = [&](float x, float y, float w, float h, float r, float g, float b, float a) {
        // Convert screen coordinates to normalized device coordinates
        float ndcX = (x / screenWidth) * 2.0f - 1.0f;
        float ndcY = 1.0f - (y / screenHeight) * 2.0f;  // Flip Y
        float ndcW = (w / screenWidth) * 2.0f;
        float ndcH = (h / screenHeight) * 2.0f;

        // Set color uniform (simplified - no model matrix)
        GLint colorLoc = glGetUniformLocation(shaderId, "color");
        GLint alphaLoc = glGetUniformLocation(shaderId, "alpha");
        glUniform3f(colorLoc, r, g, b);
        glUniform1f(alphaLoc, a);

        // Create transformation (we'll do it manually since no GLM)
        // Actually, let's use the vertex shader from earlier that expects NDC coordinates directly
        // We'll update vertices directly

        float quadVertices[] = {
            ndcX, ndcY - ndcH, 0.0f,
            ndcX + ndcW, ndcY - ndcH, 0.0f,
            ndcX + ndcW, ndcY, 0.0f,
            ndcX, ndcY, 0.0f
        };

        // Update VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quadVertices), quadVertices);

        // Draw
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        };

    // Draw semi-transparent background overlay
    drawQuad(10, 10, screenWidth - 20, 290, 0.1f, 0.1f, 0.2f, 0.85f);

    // Draw title bar
    drawQuad(10, 10, screenWidth - 20, 30, 0.3f, 0.3f, 0.5f, 1.0f);

    // Draw section backgrounds
    // Camera controls
    drawQuad(20, 60, 280, 120, 0.1f, 0.2f, 0.1f, 0.5f);

    // Model controls  
    drawQuad(320, 60, 280, 120, 0.1f, 0.1f, 0.2f, 0.5f);

    // Draw section borders (as thicker lines using thin quads)
    // Camera section border
    drawQuad(18, 58, 4, 124, 0.2f, 0.8f, 0.2f, 1.0f);  // left
    drawQuad(18, 58, 284, 4, 0.2f, 0.8f, 0.2f, 1.0f);  // top
    drawQuad(298, 58, 4, 124, 0.2f, 0.8f, 0.2f, 1.0f); // right
    drawQuad(18, 178, 284, 4, 0.2f, 0.8f, 0.2f, 1.0f); // bottom

    // Model section border
    drawQuad(318, 58, 4, 124, 0.2f, 0.2f, 0.8f, 1.0f);  // left
    drawQuad(318, 58, 284, 4, 0.2f, 0.2f, 0.8f, 1.0f);  // top
    drawQuad(598, 58, 4, 124, 0.2f, 0.2f, 0.8f, 1.0f); // right
    drawQuad(318, 178, 284, 4, 0.2f, 0.2f, 0.8f, 1.0f); // bottom

    // Draw simple key indicators (WASD keys as small boxes)
    // W key
    drawQuad(50, 90, 30, 30, 0.8f, 0.8f, 0.8f, 1.0f);

    // A key
    drawQuad(20, 130, 30, 30, 0.8f, 0.8f, 0.8f, 1.0f);

    // S key
    drawQuad(50, 130, 30, 30, 0.8f, 0.8f, 0.8f, 1.0f);

    // D key
    drawQuad(80, 130, 30, 30, 0.8f, 0.8f, 0.8f, 1.0f);

    // Restore OpenGL state
    glUseProgram(prevProgram);
    if (prevDepthTest) glEnable(GL_DEPTH_TEST);
    if (!prevBlend) glDisable(GL_BLEND);
}