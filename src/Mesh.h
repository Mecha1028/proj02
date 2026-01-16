#ifndef __MESH_H__
#define __MESH_H__

#include <iostream>
#include <vector>

#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <assimp/material.h>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;
    
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct Texture {
    GLuint id;
    std::string type;
    //std::string path;
};

// added in LabA07
struct Material {
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 ambient;
    float shininess;
};


class Mesh {

protected:
    // array of vertices and normals
    //std::vector< glm::vec3 > vertices; 

    std::vector<Vertex> vertices;

    // triangle vertex indices
    std::vector< unsigned int > indices;

    std::vector<Texture> textures;
    std::vector<Texture> normals;

    Material material;
    
    std::vector<GLuint> buffers;

    // this will be Material in the future
    GLuint shaderId;

    void initBuffer();

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, std::string dir);
    unsigned int loadTextureAndBind(const char* path, const std::string& directory);
    Material Mesh::loadMaterial(aiMaterial* mat);

public:
    Mesh();
    ~Mesh();

    void init(std::string path, GLuint shaderId);
    void loadModel(std::string path);

    void setShaderId(GLuint sid);
    
    void draw(glm::mat4 matModel, glm::mat4 matView, glm::mat4 matProj);
};

#endif