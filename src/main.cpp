#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include "shader.h"
#include "Mesh.h"
//#include "Node.h"

#include "ArcballCamera.h"


static Shader shader;

glm::mat4 matModelRoot = glm::mat4(1.0);
// glm::mat4 matView = glm::mat4(1.0);
// glm::mat4 matProj = glm::ortho(-2.0f,2.0f,-2.0f,2.0f, -2.0f,2.0f);

glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 10.0f);
glm::vec3 viewPos_default = glm::vec3(0.0f, 2.0f, 6.0f);
// glm::vec3 viewPos = viewPos_default;

// We are using mesh list instead of scenegraph to demo our picking and collision detection
std::vector< std::shared_ptr <Mesh> > meshList;
std::vector< glm::mat4 > meshMatList;

// GLuint flatShader;
GLuint blinnShader;
GLuint phongShader;
// added for LabA07
GLuint texblinnShader;

// LabA 12 Interaction Arcball camera
std::shared_ptr<ArcballCamera> camera;

bool leftDown = false;
bool middleDown = false;

double lastX, lastY;

int width = 800;
int height = 800;

// Initialize shader
GLuint initShader(std::string pathVert, std::string pathFrag) 
{
    shader.read_source( pathVert.c_str(), pathFrag.c_str());

    shader.compile();
    glUseProgram(shader.program);

    return shader.program;
}

void setLightPosition(glm::vec3 lightPos)
{
    GLuint lightpos_loc = glGetUniformLocation(shader.program, "lightPos" );
    glUniform3fv(lightpos_loc, 1, glm::value_ptr(lightPos));
}

void setViewPosition(glm::vec3 eyePos)
{
    GLuint viewpos_loc = glGetUniformLocation(shader.program, "viewPos" );
    glUniform3fv(viewpos_loc, 1, glm::value_ptr(eyePos));
}


glm::vec3 screenPosToRay(int mouseX, int mouseY, int w, int h,
                         const glm::mat4 &proj, const glm::mat4 &view);

void mouse_button_callback(GLFWwindow *win, int button, int action, int mods);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);


glm::vec3 screenPosToRay(int mouseX, int mouseY, int w, int h,
                         const glm::mat4 &proj, const glm::mat4 &view)
{
    float x = (2.0f * mouseX) / w - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / h;

    // clicked point on the near plane in NDC space
    // if we assume the eye space is scaled so that z_{near} = -1.0
    // then this NDC coordinate is the same as its clip coordinate
    glm::vec4 ray_clip(x, y, -1.0f, 1.0f);

    // set one point with (x, y) and z = -1.0 in eye/camera space
    // the camera is located at (0, 0, 0)
    glm::vec4 ray_eye = glm::inverse(proj) * ray_clip;

    // ray direction in scaled eye space: z_{near} = -1.0
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    // convert the vector to the word space
    glm::vec3 ray_world = glm::normalize(glm::vec3(glm::inverse(view) * ray_eye));
    return ray_world;
}

void mouse_button_callback(GLFWwindow *win, int button, int action, int mods)
{
    //std::cout << "Mouse click at: (" << mx <<", " << my << ")" << std::endl;

    double mx, my;
    glfwGetCursorPos(win, &mx, &my);

    int w, h;
    glfwGetWindowSize(win, &w, &h);

    // for arcball control
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftDown = true;
            camera->MouseDown(mx / w, my / h);
        } else {
            leftDown = false;
            camera->MouseUp();
        }
    }

    lastX = mx;
    lastY = my;

    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS) {
            middleDown = true;
        } else {
            middleDown = false;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        glm::vec3 rayOrig = camera->eye;
        glm::vec3 rayDir = screenPosToRay((int)mx, (int)my, w, h, camera->matProj, camera->matView);

        Ray ray{rayOrig, rayDir};

        
        for (std::shared_ptr<Mesh> pMesh : meshList)
        {
            HitInfo hit;
            if (pMesh->pSpatial->Raycast(ray, hit)) {
                std::cout << "Picked triangle index: " << hit.triIndex << ", t=" << hit.t << std::endl;
                pMesh->setPicked(true);
            } else {
                pMesh->setPicked(false);
                //std::cout << "No objects picked" << std::endl;
            }
        }
    }
}

void CursorPosCallback(GLFWwindow* window, double x, double y)
{
    //std::cout << "Cursor Pos Callback" << std::endl;

    double nx = x / width;
    double ny = y / height;

    if (leftDown) {
        camera->MouseMove(nx, ny);
    }

    if (middleDown) {
        float dx = float(x - lastX);
        float dy = float(y - lastY);
        camera->Pan(dx, dy);
    }

    lastX = x;
    lastY = y;
}


void ScrollCallback(GLFWwindow* window, double xoff, double yoff)
{
    camera->Zoom((float)yoff);
}

int main()
{
    GLFWwindow *window;

    // GLFW init
    if (!glfwInit())
    {
        std::cout << "glfw failed" << std::endl;
        return -1;
    }

    // create a GLFW window
    window = glfwCreateWindow(width, height, "Hello OpenGL 11", NULL, NULL);
    glfwMakeContextCurrent(window);


    camera = std::make_shared<ArcballCamera>(
        viewPos_default,
        glm::vec3(0,0,0), // target
        5.0f,             // distance
        60.0f,            // FOV
        float(width) / float(height),
        0.5f,
        20.0f
    );

    // register the key event callback function
    glfwSetKeyCallback(window, key_callback);
    
    // register the mouse button event callback function
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    // loading glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Couldn't load opengl" << std::endl;
        glfwTerminate();
        return -1;
    }

    phongShader = initShader( "shaders/blinn.vert", "shaders/phong.frag");
    setLightPosition(lightPos);
    setViewPosition(camera->eye);
    blinnShader = initShader( "shaders/blinn.vert", "shaders/blinn.frag");
    setLightPosition(lightPos);
    setViewPosition(camera->eye);
    // added for LabA07
    texblinnShader = initShader("shaders/texblinn.vert", "shaders/texblinn.frag");
    setLightPosition(lightPos);
    setViewPosition(camera->eye);

    // set the eye at (0, 0, 5), looking at the centre of the world
    // try to change the eye position
    //viewPos = glm::vec3(0.0f, 2.0f, 5.0f);
    // matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); 

    // set the Y field of view angle to 60 degrees, width/height ratio to 1.0, and a near plane of 3.5, far plane of 6.5
    // try to play with the FoV
    //matProj = glm::perspective(glm::radians(60.0f), 1.0f, 2.0f, 8.0f);
    // setting to a close near plane and a farway far plane to test collision detection
    // matProj = glm::perspective(glm::radians(60.0f), 1.0f, 0.5f, 20.0f);

    // matView = camera->matView;
    // matProj = camera->matProj;

    //----------------------------------------------------
    // Meshes
    std::shared_ptr<Mesh> cube = std::make_shared<Mesh>();
    cube->init("models/cube.obj", blinnShader);

    glm::mat4 mat = glm::mat4(1.0);

    
    std::shared_ptr<Mesh> teapot = std::make_shared<Mesh>();
    teapot->init("models/teapot.obj", texblinnShader);
    meshList.push_back(teapot);
    mat = glm::translate(glm::vec3(-2.0f, 1.0f, 0.0f));
    meshMatList.push_back(mat); // TRS
    teapot->initSpatial(true, mat);
    
    std::shared_ptr<Mesh> bunny = std::make_shared<Mesh>();
    bunny->init("models/bunny_normal.obj", texblinnShader);
    meshList.push_back(bunny);
    mat = glm::translate(glm::vec3(1.5f, 1.5f, 0.0f)) *
          glm::scale(glm::vec3(0.005f, 0.005f, 0.005f));
    meshMatList.push_back( mat ); // TRS
    bunny->initSpatial(true, mat);
  

    //----------------------------------------------------
    // Nodes
    // std::shared_ptr<Node> scene = std::make_shared<Node>();
    // std::shared_ptr<Node> teapotNode = std::make_shared<Node>();
    // std::shared_ptr<Node> cubeNode = std::make_shared<Node>();
    // std::shared_ptr<Node> bunnyNode = std::make_shared<Node>();
    
    //----------------------------------------------------
    // Build the tree
    //teapotNode->addMesh(teapot);
    //cubeNode->addMesh(cube, glm::mat4(1.0), glm::mat4(1.0), glm::scale(glm::vec3(2.0f, 0.25f, 1.5f)));
    //bunnyNode->addMesh(bunny, glm::mat4(1.0), glm::mat4(1.0), glm::scale(glm::vec3(0.005f, 0.005f, 0.005f)));


    // cubeNode->addChild(teapotNode, glm::translate(glm::vec3(-1.5f, 0.5f, 0.0f)));
    // cubeNode->addChild(bunnyNode, glm::translate(glm::vec3(1.0f, 1.5f, 0.0f)));
    // cubeNode->addChild(teapotNode, glm::translate(glm::vec3(0.0f, 1.0f, 0.0f)), glm::rotate(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    
    //----------------------------------------------------
    // Add the tree to the world space
    //scene->addChild(cubeNode);
    //scene->addChild(bunnyNode);
    // scene->addChild(cubeNode, glm::translate(glm::vec3(1.0f, 0.0f, 0.0f)), glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)));

    // setting the background colour, you can change the value
    glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
    
    glEnable(GL_DEPTH_TEST);
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    // setting the event loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //scene->draw(matModelRoot, matView, matProj);
        // bunny->draw(glm::scale(glm::vec3(0.005f, 0.005f, 0.005f)), matView, matProj);


        for (int i = 0; i < meshList.size(); i++ ) {
            std::shared_ptr<Mesh> pMesh = meshList[i];
            pMesh->draw(matModelRoot * meshMatList[i], camera->matView, camera->matProj);
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();

    return 0;
}


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    glm::mat4 mat = glm::mat4(1.0);

    float angleStep = 5.0f;
    float transStep = 1.0f;

    if (action == GLFW_PRESS)
    {
        /*
        // we don't allow objects to move for picking and collision detection
        if (mods & GLFW_MOD_CONTROL) {
            // translation in world space
            if (GLFW_KEY_LEFT == key) {
                mat = glm::translate(glm::mat4(1.0f), glm::vec3(transStep, 0.0f, 0.0f));
                matModelRoot = mat * matModelRoot;
            } else if (GLFW_KEY_RIGHT == key) {
                mat = glm::translate(glm::mat4(1.0f), glm::vec3(-transStep, 0.0f, 0.0f));
            } else if (GLFW_KEY_UP == key) {
                mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, transStep, 0.0f));
                matModelRoot = mat * matModelRoot;
            } else if (GLFW_KEY_DOWN == key) {
                mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -transStep, 0.0f));
            }
            matModelRoot = mat * matModelRoot;
        }
        */

        if (GLFW_KEY_R == key)
        {
            // std::cout << "R pressed" << std::endl;
            //  reset
            // viewPos = viewPos_default;
            // matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

            camera->reset(viewPos_default, glm::vec3(0, 0, 0));
            matModelRoot = glm::mat4(1.0f);

            return;
        } 

        glm::mat4 nextMatView = camera->matView;
        glm::vec3 nextViewPos = camera->eye;

        // camera control
        if (mods & GLFW_MOD_CONTROL) {
            if (GLFW_KEY_LEFT == key) {
                // pan left, rotate around Y, CCW
                mat = glm::rotate(glm::radians(-angleStep), glm::vec3(0.0, 1.0, 0.0));
                nextMatView = mat * camera->matView;
            }
            else if (GLFW_KEY_RIGHT == key) {
                // pan right, rotate around Y, CW
                mat = glm::rotate(glm::radians(angleStep), glm::vec3(0.0, 1.0, 0.0));
                nextMatView = mat * camera->matView;
            }
            else if (GLFW_KEY_UP == key) {
                // tilt up, rotate around X, CCW
                mat = glm::rotate(glm::radians(-angleStep), glm::vec3(1.0, 0.0, 0.0));
                nextMatView = mat * camera->matView;
            }
            else if (GLFW_KEY_DOWN == key) {
                // tilt down, rotate around X, CW
                mat = glm::rotate(glm::radians(angleStep), glm::vec3(1.0, 0.0, 0.0));
                nextMatView = mat * camera->matView;
            }
            else if ((GLFW_KEY_KP_ADD == key) ||
                (GLFW_KEY_EQUAL == key) && (mods & GLFW_MOD_SHIFT)) {
                // zoom in, move along -Z
                mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, transStep));
                nextMatView = mat * camera->matView;
            }
            else if ((GLFW_KEY_KP_SUBTRACT == key) || (GLFW_KEY_MINUS == key)) {
                // zoom out, move along -Z
                mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -transStep));
                nextMatView = mat * camera->matView;
            }
        }

        // translation along camera axis (first person view)
        if (GLFW_KEY_A == key) {
            //  move left along -X
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(transStep, 0.0f, 0.0f));
            nextMatView = mat * camera->matView;
        } else if (GLFW_KEY_D == key) {
            // move right along X
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(-transStep, 0.0f, 0.0f));
            nextMatView = mat * camera->matView;
        } else if (GLFW_KEY_W == key) {
            // move forward along -Z
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, transStep));
            nextMatView = mat * camera->matView;
        } else if (GLFW_KEY_S == key) {
            // move backward along Z
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -transStep)); 
            nextMatView = mat * camera->matView;
        }


        // translation along world axis
        if (GLFW_KEY_LEFT == key) {
            //  move left along -X
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(transStep, 0.0f, 0.0f));
            nextMatView = camera->matView * mat;
            nextViewPos.x -= transStep;
        } else if (GLFW_KEY_RIGHT == key) {
            // move right along X
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(-transStep, 0.0f, 0.0f));
            nextMatView = camera->matView * mat;
            nextViewPos.x += transStep;
        } else if (GLFW_KEY_UP == key) {
            // move up along Y
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -transStep, 0.0f));
            nextMatView = camera->matView * mat;
            nextViewPos.y += transStep;
        } else if (GLFW_KEY_DOWN == key) {
            // move down along -Y
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, transStep, 0.0f));
            nextMatView = camera->matView * mat;
            nextViewPos.y -= transStep;
        } else  if ((GLFW_KEY_KP_SUBTRACT == key) || (GLFW_KEY_MINUS == key))  {
            // move backward along +Z
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -transStep));
            nextMatView = camera->matView * mat;
            nextViewPos.z += transStep;
        } else if ((GLFW_KEY_KP_ADD == key) ||
            (GLFW_KEY_EQUAL == key) && (mods & GLFW_MOD_SHIFT))  {
            // move forward along -Z
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, transStep));
            nextMatView = camera->matView * mat;
            nextViewPos.z -= transStep;
        }


        // check collision detection
        AABB mybox{ nextViewPos - 0.2f, nextViewPos + 0.2f };
        std::vector<int> out;

        bool bCollide = false;
        for (std::shared_ptr<Mesh> pMesh : meshList)
        {
            pMesh->pSpatial->QueryAABB(mybox, out);
            
            if (out.empty()) {
                std::cout << "No collision" << std::endl;
            } else {
                bCollide = true;
                std::cout << "Collision detected: " << out.size() << std::endl;
            }
        }

        if (!bCollide) {
            camera->matView = nextMatView;
            // should use the camera class
            camera->eye = nextViewPos;
        }
    }
}