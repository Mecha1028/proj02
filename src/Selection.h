#ifndef __SELECTIONSYSTEM_H__
#define __SELECTIONSYSTEM_H__

#include <vector>
#include <memory>
#include <glm/glm.hpp>

class Node;

class Selection {
private:
    int selectedIndex = -1;  // -1 = none selected
    std::vector<std::shared_ptr<Node>> selectableNodes;
    glm::vec3 selectedOffset = glm::vec3(0.0f);

    std::vector<glm::vec3> originalPositions;

public:
    Selection();

    void addSelectable(std::shared_ptr<Node> node);
    void select(int index);
    void moveSelected(float dx, float dy, float dz);
    void applyTransformations();

    int getSelectedIndex() const { return selectedIndex; }
    glm::vec3 getSelectedOffset() const { return selectedOffset; }

    // For debugging
    void printStatus();
};

#endif