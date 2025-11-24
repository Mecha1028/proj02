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
#include "Node.h"

static Shader shader;

bool bDepth = false;

glm::mat4 matModelRoot = glm::mat4(1.0);
glm::mat4 matView = glm::mat4(1.0);
glm::mat4 matProj = glm::ortho(-2.0f,2.0f,-2.0f,2.0f, -2.0f,2.0f);

glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 5.0f);
glm::vec3 viewPos = glm::vec3(0.0f, 0.0f, 5.0f);

GLuint blinnShader;
GLuint phongShader;
GLuint texblinnShader;
//GLuint normalblinnShader;

// ========================================
// for LabA08 Shadow Mapping
int shadowMapWidth = 1024;
int shadowMapHeight = 1024;

// Window Viewport Size. for demo purpose we set shadow size equal to window size
// should be in a higher resolution
int width = shadowMapWidth;
int height = shadowMapHeight;
// int width = 800;
// int height = 800;

GLuint depthTex;  // depth texture ID
GLuint shadowFBO; // shadow frame buffer ID

GLuint depthShader;  // depth shader program ID
GLuint shadowShader; // shadow map shader program ID

glm::mat4 matLightView; // view matrix from the light source
glm::mat4 matLightProj; // projection matrix from the light source


void initRenderToDepthTexture()
{
    //GLfloat border[] = {1.0f, 0.0f,0.0f,0.0f };
    
    // Assign the depth buffer texture to texture channel 1
    glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, depthTex);

    // Generate the depth texture ID
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    
    // set texture size and format, high resolution 24bit for depth
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, shadowMapWidth, shadowMapHeight);

    // set Interpolation and Out-of-range texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);


    // Render to Texture
    // Create and set up the FBO
    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

    // use the depth texture as the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, depthTex, 0);

    //GLenum drawBuffers[] = {GL_NONE};
    //glDrawBuffers(1, drawBuffers);
    // set draw buffer to GL_NONE
    glDrawBuffer(GL_NONE);
    //glReadBuffer(GL_NONE);

    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( result == GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer is complete.\n");
    } else {
        printf("Framebuffer is not complete.\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

// ========================================
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

void window_size_callback(GLFWwindow* window, int w, int h)
{
    //int width, height;
    //glfwGetWindowSize(window, &width, &height);

    width = w;
    height = h;

    glViewport(0, 0, width, height);

    matProj = glm::perspective(glm::radians(60.0f), width/(float)height, 2.0f, 8.0f);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    glm::mat4 mat = glm::mat4(1.0);

    float angleStep = 5.0f;
    float transStep = 1.0f;

    if (action == GLFW_PRESS) {

        if (mods & GLFW_MOD_CONTROL) {
            if (GLFW_KEY_D == key) {
                bDepth = !bDepth;
                return;
            }

            // translation in world space
            if (GLFW_KEY_LEFT == key) {
                mat = glm::translate(glm::mat4(1.0f), glm::vec3(transStep, 0.0f, 0.0f));
                matModelRoot = mat * matModelRoot;
            } else if (GLFW_KEY_RIGHT == key ) {
                mat = glm::translate(glm::mat4(1.0f), glm::vec3(-transStep, 0.0f, 0.0f));
                matModelRoot = mat * matModelRoot;
            } else if (GLFW_KEY_UP == key) {
                mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, transStep, 0.0f));
                matModelRoot = mat * matModelRoot;
            } if (GLFW_KEY_DOWN == key) {
                mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -transStep, 0.0f));
                matModelRoot = mat * matModelRoot;
            }
        }

        // camera control
        if (GLFW_KEY_LEFT == key) {
            // pan left, rotate around Y, CCW
            mat = glm::rotate(glm::radians(-angleStep), glm::vec3(0.0, 1.0, 0.0));
            matView = mat * matView;
        } else if (GLFW_KEY_RIGHT == key ) {
            // pan right, rotate around Y, CW
            mat = glm::rotate(glm::radians(angleStep), glm::vec3(0.0, 1.0, 0.0));
            matView = mat * matView;
        } else if (GLFW_KEY_UP == key) {
            // tilt up, rotate around X, CCW
            mat = glm::rotate(glm::radians(-angleStep), glm::vec3(1.0, 0.0, 0.0));
            matView = mat * matView;
        } if (GLFW_KEY_DOWN == key) {
            // tilt down, rotate around X, CW
            mat = glm::rotate(glm::radians(angleStep), glm::vec3(1.0, 0.0, 0.0));
            matView = mat * matView;
        } else if ( (GLFW_KEY_KP_ADD == key) || 
            (GLFW_KEY_EQUAL == key) && (mods & GLFW_MOD_SHIFT) ) {
            // std::cout << "+ pressed" << std::endl;
            // zoom in, move along -Z
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, transStep));
            matView = mat * matView ;
        } else if ( (GLFW_KEY_KP_SUBTRACT == key ) || (GLFW_KEY_MINUS == key) ) {
            // std::cout << "keypad - pressed" << std::endl;
            // zoom out, move along -Z
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -transStep));
            matView = mat * matView;
        } else if (GLFW_KEY_R == key) {
            //std::cout << "R pressed" << std::endl;
            // reset
            matView = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); 
            matModelRoot = glm::mat4(1.0f);
        }

        // translation along camera axis (first person view)
        else if (GLFW_KEY_A == key ) {
            //if (modes & GLFW_MOD_CONTROL)
            // move left along -X
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(transStep, 0.0f, 0.0f));
            matView = mat * matView;
        } else if (GLFW_KEY_D == key) {
            // move right along X
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(-transStep, 0.0f, 0.0f));
            matView = mat * matView;
        } if (GLFW_KEY_W == key ) {
            // move forward along -Z
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, transStep));
            matView = mat * matView;
        } else if (GLFW_KEY_S == key) {
            // move backward along Z
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -transStep));
            matView = mat * matView;
        } 

        // translation along world axis
        else if (GLFW_KEY_H == key ) {
            //if (modes & GLFW_MOD_CONTROL)
            // move left along -X
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(transStep, 0.0f, 0.0f));
            matView = matView * mat;
        } else if (GLFW_KEY_L == key) {
            // move right along X
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(-transStep, 0.0f, 0.0f));
            matView = matView * mat;
        } if (GLFW_KEY_J == key ) {
            // move forward along Z
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -transStep));
            matView = matView * mat;
        } else if (GLFW_KEY_K == key) {
            // move backward along -Z
            mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, transStep));
            matView = matView * mat;
        } 
    }
    
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
    window = glfwCreateWindow(width, height, "Hello OpenGL 9", NULL, NULL);
    glfwMakeContextCurrent(window);

    // register the key event callback function
    glfwSetKeyCallback(window, key_callback);

    glfwSetWindowSizeCallback(window, window_size_callback);


    // loading glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Couldn't load opengl" << std::endl;
        glfwTerminate();
        return -1;
    }

    // flatShader = initShader( "shaders/flat.vert", "shaders/flat.frag");
    // initLightPosition(lightPos);
    phongShader = initShader( "shaders/blinn.vert", "shaders/phong.frag");
    setLightPosition(lightPos);
    setViewPosition(viewPos);

    blinnShader = initShader( "shaders/blinn.vert", "shaders/blinn.frag");
    setLightPosition(lightPos);
    setViewPosition(viewPos);

    texblinnShader = initShader("shaders/texblinn.vert", "shaders/texblinn.frag");
    setLightPosition(lightPos);
    setViewPosition(viewPos);
    //setLightPosition(glm::vec3(5.0f, 5.0f, 10.0f));
    //setViewPosition(lightPos);

    //normalblinnShader = initShader("shaders/normalblinn2.vert", "shaders/normalblinn2.frag");
    //setLightPosition(lightPos);
    //setViewPosition(viewPos);

    // LabA09 Shadow Map
    depthShader = initShader("shaders/simpledepth.vert", "shaders/simpledepth.frag");
    //setLightPosition(lightPos);
    //setViewPosition(viewPos);

    shadowShader = initShader("shaders/shadowmap.vert", "shaders/shadowmap.frag");
    setLightPosition(lightPos);
    setViewPosition(viewPos);


    // set the eye position, looking at the centre of the world
    viewPos = glm::vec3(0.0f, 2.0f, 5.0f);
    matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); 

    // set the Y field of view angle to 60 degrees, width/height ratio to 1.0, and a near plane of 3.5, far plane of 6.5
    matProj = glm::perspective(glm::radians(60.0f), width / (float) height, 2.0f, 8.0f);

    // LabA09 Shadow Map
    matLightView = glm::lookAt(lightPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); 
    // matLightProj = glm::perspective(glm::radians(60.0f), 1.0f, 2.0f, 8.0f);
    matLightProj = glm::ortho(-3.0f,3.0f,-3.0f,3.0f, -5.0f,15.0f);

    //glUseProgram(depthShader);
    glm::mat4 matLightviewproj = matLightProj * matLightView;
    GLuint loc = glGetUniformLocation(shadowShader, "lightSpaceMatrix" );
    glUniformMatrix4fv(loc, 1, GL_FALSE, &matLightviewproj[0][0]);
    GLuint textureLoc = glGetUniformLocation(shadowShader, "shadowMap");
    glUniform1i(textureLoc, 1);

    //----------------------------------------------------
    // Meshes
    std::shared_ptr<Mesh> cube = std::make_shared<Mesh>();
    cube->init("models/cube.obj", blinnShader);

    std::shared_ptr<Mesh> teapot = std::make_shared<Mesh>();
    teapot->init("models/teapot.obj", blinnShader);

    std::shared_ptr<Mesh> bunny = std::make_shared<Mesh>();
    bunny->init("models/bunny_normal.obj", texblinnShader);

    //std::shared_ptr<Mesh> box = std::make_shared<Mesh>();
    // box->init("models/Box_normal.obj", normalblinnShader);

    
    //----------------------------------------------------
    // Nodes
    std::shared_ptr<Node> scene = std::make_shared<Node>();
    std::shared_ptr<Node> teapotNode = std::make_shared<Node>();
    std::shared_ptr<Node> cubeNode = std::make_shared<Node>();
    std::shared_ptr<Node> bunnyNode = std::make_shared<Node>();
    //std::shared_ptr<Node> boxNode = std::make_shared<Node>();
    
    //----------------------------------------------------
    // Build the tree
    teapotNode->addMesh(teapot);
    cubeNode->addMesh(cube, glm::mat4(1.0), glm::mat4(1.0), glm::scale(glm::vec3(2.0f, 0.25f, 1.5f)));
    bunnyNode->addMesh(bunny, glm::mat4(1.0), glm::mat4(1.0), glm::scale(glm::vec3(0.005f, 0.005f, 0.005f)));
    //boxNode->addMesh(box, glm::mat4(1.0), glm::rotate(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

    cubeNode->addChild(teapotNode, glm::translate(glm::vec3(-1.2f, 0.5f, 0.0f)));
    cubeNode->addChild(bunnyNode, glm::translate(glm::vec3(1.0f, 1.5f, 0.5f)));
    // cubeNode->addChild(teapotNode, glm::translate(glm::vec3(0.0f, 1.0f, 0.0f)), glm::rotate(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    
    //----------------------------------------------------
    // Add the tree to the world space
    scene->addChild(cubeNode);
    // scene->addChild(boxNode);
    // scene->addChild(cubeNode, glm::translate(glm::vec3(1.0f, 0.0f, 0.0f)), glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)));

    // setting the background colour, you can change the value
    glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
    
    glEnable(GL_DEPTH_TEST);

    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // LabA09 Shadow Map
    initRenderToDepthTexture();

    // setting the event loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        
        // 1st pass, draw the shadow depth map
        if (! bDepth)
            glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
        
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        glViewport(0,0,shadowMapWidth, shadowMapHeight);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        //glEnable(GL_POLYGON_OFFSET_FILL);
        //glPolygonOffset(0.5f,0.5f);

        scene->setShaderId(depthShader);
        scene->draw(matModelRoot, matLightView, matLightProj);
        
        glCullFace(GL_BACK);
        glFlush();

        
        // 2nd pass
        if (!bDepth)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, width, height);

            scene->setShaderId(shadowShader);
            scene->draw(matModelRoot, matView, matProj);
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();

    return 0;
}