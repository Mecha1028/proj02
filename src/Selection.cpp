#include "Selection.h"
#include "Node.h"
#include <iostream>

Selection::Selection() : selectedIndex(-1) {}

void Selection::addSelectable(std::shared_ptr<Node> node) {
    selectableNodes.push_back(node);
    originalPositions.push_back(glm::vec3(0.0f));
}

void Selection::select(int index) {
    if (index >= 0 && index < selectableNodes.size()) {
        selectedIndex = index;
        std::cout << "Selected object: " << index << std::endl;
    }
    else {
        selectedIndex = -1;
        std::cout << "Deselected all objects" << std::endl;
    }
}

void Selection::moveSelected(float dx, float dy, float dz) {
    if (selectedIndex >= 0 && selectedIndex < selectableNodes.size()) {
        selectedOffset.x += dx;
        selectedOffset.y += dy;
        selectedOffset.z += dz;

        std::cout << "Selected object offset: "
            << selectedOffset.x << ", "
            << selectedOffset.y << ", "
            << selectedOffset.z << std::endl;
    }
}

void Selection::applyTransformations() {
    if (selectedIndex >= 0 && selectedIndex < selectableNodes.size()) {
        // Create translation matrix from OFFSET (not absolute position)
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), selectedOffset);

        // Apply offset to the selected node
        selectableNodes[selectedIndex]->applyOffset(selectedOffset);

        std::cout << "Applied offset to object " << selectedIndex
            << ": (" << selectedOffset.x << ", "
            << selectedOffset.y << ", " << selectedOffset.z << ")" << std::endl;
    }
    selectedOffset = glm::vec3(0.0f);
}

void Selection::printStatus() {
    std::cout << "Selection System Status:" << std::endl;
    std::cout << "  Selected index: " << selectedIndex << std::endl;
    std::cout << "  Selectable nodes: " << selectableNodes.size() << std::endl;
    std::cout << "  Current offset: " << selectedOffset.x << ", "
        << selectedOffset.y << ", " << selectedOffset.z << std::endl;
}