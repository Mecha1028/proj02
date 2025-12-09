#ifndef __ARCBALLCAMERA_H__
#define __ARCBALLCAMERA_H__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

// 2025.12.09: we should have a separate Camera class in the future

class ArcballCamera {
private:
    glm::quat rotation;
    glm::vec3 lastPos;
    bool dragging = false;
public:
    glm::vec3 eye;
    glm::vec3 target;
    glm::vec3 up;

    float distance;
    float fov, aspect;
    float near;
    float far;

    glm::mat4 matView;
    glm::mat4 matProj;


public:
    ArcballCamera(glm::vec3 e, glm::vec3 target, float distance, float fov, float aspect, float n, float f)
        : eye(e), target(target), distance(distance), fov(fov), aspect(aspect), near(n), far(f)
    {
        rotation = glm::quat(1,0,0,0); 
        matProj = glm::perspective(glm::radians(fov), aspect, near, far);
        UpdateView();
    }

    void MouseDown(double x, double y) {
        dragging = true;
        lastPos = ScreenToArcball(x, y);
    }

    void MouseUp() {
        dragging = false;
    }

    void MouseMove(double x, double y) {
        if (!dragging) return;

        glm::vec3 currPos = ScreenToArcball(x, y);

        // Compute quaternion between last and current
        glm::vec3 axis = glm::cross(lastPos, currPos);
        float dot = glm::dot(lastPos, currPos);
        float angle = -acos(glm::min(1.0f, glm::max(-1.0f, dot)));

        glm::quat dq = glm::angleAxis(angle, glm::normalize(axis));
        rotation = dq * rotation;

        lastPos = currPos;

        UpdateView();
    }

    // Zoom with scroll wheel
    void Zoom(float delta) {
        distance *= (1.0f - delta * 0.1f);
        if (distance < 0.1f) distance = 0.1f;
        UpdateView();
    }

    // Middle-mouse pan
    void Pan(float dx, float dy) {
        glm::vec3 right = glm::vec3(rotation * glm::vec3(1,0,0));
        glm::vec3 up    = glm::vec3(rotation * glm::vec3(0,1,0));

        target += -right * dx * distance * 0.001f;
        target +=  up    * dy * distance * 0.001f;
        UpdateView();
    }

    void setEye(glm::vec3 e) { eye = e; }
    void setTarget(glm::vec3 t) {target = t; }

    void reset(glm::vec3 e, glm::vec3 t, glm::vec3 u = glm::vec3(0, 1, 0)) {
        setEye(e);
        setTarget(t);
        up = u;

        rotation = glm::quat(1,0,0,0); 
        // UpdateView();
        matView = glm::lookAt(eye, target, glm::vec3(0,1,0));
    }

    const glm::mat4& GetView()     const { return matView; }
    const glm::mat4& GetProjection() const { return matProj; }

    void SetViewportSize(float width, float height) {
        aspect = width / height;
        matProj = glm::perspective(glm::radians(fov), aspect, 0.01f, 1000.0f);
    }

private:

    glm::vec3 ScreenToArcball(double x, double y) {
        // Convert to [-1,1] range
        float nx = (2.0f * float(x)) - 1.0f;
        float ny = 1.0f - (2.0f * float(y));

        glm::vec2 p(nx, ny);
        float len2 = glm::dot(p, p);

        if (len2 <= 1.0f) {
            float z = sqrtf(1.0f - len2);
            return glm::vec3(p, z);
        } else {
            glm::vec2 norm = glm::normalize(p);
            return glm::vec3(norm, 0.0f);
        }
    }

    void UpdateView() {
        glm::vec3 forward = rotation * glm::vec3(0,0,-1);
        eye = target - forward * distance;

        matView = glm::lookAt(eye, target, rotation * glm::vec3(0,1,0));
    }
};

#endif