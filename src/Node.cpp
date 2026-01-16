#include "Node.h"
#include <glm/gtx/matrix_decompose.hpp>

void Node::addChild(std::shared_ptr<Node> child, glm::mat4 trans, glm::mat4 rot )
{
    childNodes.push_back(child);
    childMats.push_back(trans * rot);
}

void Node::addMesh(std::shared_ptr<Mesh> mesh, glm::mat4 trans, glm::mat4 rot, glm::mat4 scale)
{
    meshes.push_back(mesh);
    meshMats.push_back(trans * rot * scale);
}

void Node::setShaderId(GLuint sid) {
    for (auto & m : meshes) {
        m->setShaderId(sid);
    }

    for (auto & n : childNodes) {
        n->setShaderId(sid);
    }
}

// all drawings go to Mesh::draw()
void Node::draw(glm::mat4 matModel, glm::mat4 matView, glm::mat4 matProj)
{
    //std::cout << "Node::draw()" << std::endl;

    for (int i = 0; i < meshes.size(); i++) {
        meshes[i]->draw(matModel * meshMats[i], matView, matProj);
    }

    // for (const auto& child : childNodes) {
    for (int i = 0; i < childNodes.size(); i++) {
        childNodes[i]->draw(matModel * childMats[i], matView, matProj);
    }
}

void Node::setLocalTransform(glm::mat4 newTranslation) {
    // newTranslation should be JUST a translation matrix
    // Extract the translation vector from the new matrix
    glm::vec3 translation = glm::vec3(newTranslation[3]);

    // Apply this translation to ALL meshes
    for (int i = 0; i < meshMats.size(); i++) {
        // Extract scale and rotation from current matrix
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 oldTranslation;
        glm::vec3 skew;
        glm::vec4 perspective;

        // Decompose current matrix
        glm::decompose(meshMats[i], scale, rotation, oldTranslation, skew, perspective);

        // Rebuild with new translation, but keep scale and rotation
        meshMats[i] = glm::translate(translation) * glm::mat4_cast(rotation) * glm::scale(scale);
    }

    // Also apply to children
    for (int i = 0; i < childMats.size(); i++) {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 oldTranslation;
        glm::vec3 skew;
        glm::vec4 perspective;

        glm::decompose(childMats[i], scale, rotation, oldTranslation, skew, perspective);
        childMats[i] = glm::translate(translation) * glm::mat4_cast(rotation) * glm::scale(scale);
    }
}

void Node::applyOffset(glm::vec3 offset) {
    // Apply offset to all mesh matrices
    for (int i = 0; i < meshMats.size(); i++) {
        meshMats[i] = glm::translate(offset) * meshMats[i];
    }

    // Apply offset to all child matrices
    for (int i = 0; i < childMats.size(); i++) {
        childMats[i] = glm::translate(offset) * childMats[i];
    }
}