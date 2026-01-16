#include <iostream>

#include <glad/glad.h>

#include "Mesh.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


Mesh::Mesh()
{

}

Mesh::~Mesh()
{

}

void Mesh::init(std::string path, GLuint id)
{
    shaderId = id;
    loadModel(path);
    initBuffer();
}

void Mesh::loadModel(std::string path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace /* | aiProcess_GenNormals */);
    if (NULL != scene) {
        std::cout << "load model successful" << std::endl;
    }
    else {
        std::cout << "load model failed" << std::endl;
    }

    // mNumMeshes should > 0
    // std::cout << scene->mNumMeshes << std::endl;

    // LabA07
    Vertex v;

    // at the moment we only handle one mesh
    for (int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[i];

        // read vertex position and normals
        int nVertex = mesh->mNumVertices;
        // std::cout << mesh->mNumVertices << std::endl;
        //std::cout << "texCoord length: " << mesh->mTextureCoords[0]->Length() << std::endl;

        for (int j = 0; j < nVertex; j++)
        {
            glm::vec3 pos;
            pos.x = mesh->mVertices[j].x;
            pos.y = mesh->mVertices[j].y;
            pos.z = mesh->mVertices[j].z;
            // vertices.push_back(pos);
            v.pos = pos;

            // std::cout << pos.x << pos.y << pos.z << std::endl;

            glm::vec3 normal;
            normal.x = mesh->mNormals[j].x;
            normal.y = mesh->mNormals[j].y;
            normal.z = mesh->mNormals[j].z;
            //vertices.push_back(normal);

            v.normal = normal;

            if (mesh->mTangents && mesh->mBitangents) {  // ADD THIS CHECK
                glm::vec3 tangent;
                tangent.x = mesh->mTangents[j].x;
                tangent.y = mesh->mTangents[j].y;
                tangent.z = mesh->mTangents[j].z;
                v.tangent = tangent;

                tangent.x = mesh->mBitangents[j].x;
                tangent.y = mesh->mBitangents[j].y;
                tangent.z = mesh->mBitangents[j].z;
                v.bitangent = tangent;
            }
            else {  // ADD DEFAULT VALUES
                v.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
                v.bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
            }


            // LabA07 Texture Coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][j].x;
                vec.y = mesh->mTextureCoords[0][j].y;
                v.texCoord = vec;
            }
            else {
                std::cout << "tex coord zero" << std::endl;
                v.texCoord = glm::vec2(0.0f, 0.0f);
            }
            vertices.push_back(v);
        }


        int nFaces = mesh->mNumFaces;
        for (int j = 0; j < nFaces; j++)
        {
            const aiFace& face = mesh->mFaces[j];
            for (int k = 0; k < 3; k++)
            {
                indices.push_back(face.mIndices[k]);
            }
        }
    }

    // at the moment
    // we only deal with one material/texture
    // loading material

    aiMesh* mesh = scene->mMeshes[0];

    if (NULL != mesh && mesh->mMaterialIndex > 0)
    {
        std::string dir = "";
        const size_t last_slash_idx = path.rfind('/');
        if (std::string::npos != last_slash_idx)
        {
            dir = path.substr(0, last_slash_idx);
        }

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
            aiTextureType_DIFFUSE, "texture_diffuse", dir);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> normalMaps = loadMaterialTextures(material,
            aiTextureType_HEIGHT, "texture_normal", dir);
        normals.insert(normals.end(), normalMaps.begin(), normalMaps.end());

        this->material = loadMaterial(material);

        // we don't deal with specular maps
        //std::vector<Texture> specularMaps = loadMaterialTextures(material, 
        //                                    aiTextureType_SPECULAR, "texture_specular", dir);
        //textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    // for debugging
    std::cout << "numVertex: " << vertices.size() << std::endl;
    std::cout << "numIndex: " << indices.size() << std::endl;
}

void Mesh::initBuffer()
{
    // create vertex buffer
    GLuint vao;
    glGenVertexArrays(1, &vao);
    GLuint vertBufID;
    glGenBuffers(1, &vertBufID);
    glBindBuffer(GL_ARRAY_BUFFER, vertBufID);
    GLuint idxBufID;
    glGenBuffers(1, &idxBufID);

    // remember VAO
    glBindVertexArray(vao);
    buffers.push_back(vao);

    std::cout << "vertBufId: " << vertBufID << std::endl;

    buffers.push_back(vertBufID);

    // set buffer data to triangle vertex and setting vertex attributes
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0] /*vertices.data()*/, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

    // set normal attributes
    glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void *) (sizeof(float) * 3));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));

    // LabA07: Adding texture coord attribute
    // vertex texture coords
    glEnableVertexAttribArray(2);
    // the second parameter: 2 coordinates (tx, ty) per texture coord	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));


    // tangent space
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));


    // bind index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxBufID);

    std::cout << "idxBufId: " << idxBufID << std::endl;
    buffers.push_back(idxBufID);

    // set buffer data for triangle index
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Mesh::setShaderId(GLuint sid) {
    shaderId = sid;
}


std::vector<Texture> Mesh::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, std::string dir)
{
    std::vector<Texture> textures;

    int nTex = mat->GetTextureCount(type);
    for (unsigned int i = 0; i < nTex; i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        Texture texture;
        texture.id = loadTextureAndBind(str.C_Str(), dir);
        texture.type = typeName;
        //texture.path = std::string(str.C_Str());
        textures.push_back(texture);
    }
    return textures;
}

unsigned int Mesh::loadTextureAndBind(const char* path, const std::string& directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    // we need flip for the bunny model
    // stbi_set_flip_vertically_on_load(true); 

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


Material Mesh::loadMaterial(aiMaterial* mat)
{
    Material material;
    aiColor3D color(0.f, 0.f, 0.f);
    float shininess;

    mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    material.diffuse = glm::vec3(color.r, color.b, color.g);

    mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
    material.ambient = glm::vec3(color.r, color.b, color.g);

    mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
    material.specular = glm::vec3(color.r, color.b, color.g);

    mat->Get(AI_MATKEY_SHININESS, shininess);
    material.shininess = shininess;

    return material;
}



// all drawings come here
void Mesh::draw(glm::mat4 matModel, glm::mat4 matView, glm::mat4 matProj)
{
    // 1. Bind the correct shader program
    glUseProgram(shaderId);

    //std::cout << "shader: " << shaderId << std::endl;

    // 2. Set the appropriate uniforms for each shader
    // set the modelling transform  
    GLuint model_loc = glGetUniformLocation(shaderId, "model");
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, &matModel[0][0]);

    // set view matrix
    GLuint view_loc = glGetUniformLocation(shaderId, "view");
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, &matView[0][0]);

    // set projection transforms
    glm::mat4 mat_projection = matProj;
    GLuint projection_loc = glGetUniformLocation(shaderId, "projection");
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, &mat_projection[0][0]);

    GLint textureLoc = glGetUniformLocation(shaderId, "textureMap");
    glUniform1i(textureLoc, 0);

    GLint hasTextureLoc = glGetUniformLocation(shaderId, "hasTexture");
    glUniform1i(hasTextureLoc, 0);

    if (!textures.empty())
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0].id);
        glUniform1i(hasTextureLoc, GL_TRUE);
    }
    else {
        glUniform1i(hasTextureLoc, GL_FALSE);
    }

    if (!normals.empty())
    {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, normals[0].id);
        GLuint hasNormalLoc = glGetUniformLocation(shaderId, "hasNormalMap");
        glUniform1i(hasNormalLoc, GL_TRUE);

        GLuint normalMapLoc = glGetUniformLocation(shaderId, "normalMap");
        glUniform1i(normalMapLoc, 2);
    }
    else {
        GLuint hasNormalLoc = glGetUniformLocation(shaderId, "hasNormalMap");
        if (hasNormalLoc != -1) {
            glUniform1i(hasNormalLoc, GL_FALSE);
        }
    }


    //if (bShadow)
    {
        GLint loc = glGetUniformLocation(shaderId, "depthMap");
        glUniform1i(loc, 1);

    }

    // 3. Bind the corresponding model's VAO
    glBindVertexArray(buffers[0]);

    // 4. Draw the model
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // 5. Unset vertex buffer
    glBindVertexArray(0);
}