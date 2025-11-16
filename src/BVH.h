#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "Mesh.h"

class Box {
public:
    glm::vec3 lowerleft;
    glm::vec3 size;

    bool intersect(Box b);
};

class Ray {
public:
    glm::vec3 start;
    glm::vec3 dir;
};

class Intersect {
public:
    std::shared_ptr<Mesh> mesh;
    std::vector<GLuint> triList;
};

class BVH {
    public:
        bool isEmpty();
        Intersect intersect(Box b);
        Intersect intersect(Ray r);
};

class GridBVH : BVH {
    public:
        float xspace;
        float yspace;
        int nX;
        int nY;
};

class Octree : BVH {
public:
    // eight children
    std::vector< std::shared_ptr<Octree> > childList;
};