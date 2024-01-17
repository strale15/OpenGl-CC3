//Opis: Primjer ucitavanja modela upotrebom ASSIMP biblioteke
//Preuzeto sa learnOpenGL

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "model.hpp"

#include "GameObject.cpp"

#include <Windows.h>
#include <cmath>

const unsigned int wWidth = 1920;
const unsigned int wHeight = 1080;

bool firstMouse = true;
double lastX;
double lastY;

struct Params {
    float dt = 0;
    bool isFps = false;

    glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

    glm::vec3 position = glm::vec3(0.0, 0.0, -1.0);
    glm::vec3 objPos = glm::vec3(0.0, 0.0, 0.0);

    bool wDown = false;
    bool sDown = false;
    bool aDown = false;
    bool dDown = false;

    //Valley
    bool modelRotating = false;
    bool isDay = true;
    float treeWhite = 0;
    float modelRot = 0;
    float cloudsZoffset = 0;
    int appleNumber = 0;
    int appState = 3;

    float isoAngle = 0;
    bool treeWhiteUpPress = false;
    bool treeWhiteDownPress = false;
    float sunScale = 1.5;
    float sunG = 0.45;
    bool sunScaleMaxReached = true;
    bool sunColorMaxReached = true;
};

static void DrawHud(Shader& hudShader, unsigned hudTex) {
    //hud
        // Bind your HUD shader program
    hudShader.use();

    // Define the vertices of a rectangle
    float vertices[] = {
       -1.0f,  1.0f, 0.0f,     0.0f, 0.0f, // Top-left vertex
       -1.0f, -1.0f, 0.0f,     0.0f, 1.0f, // Bottom-left vertex
        1.0f, -1.0f, 0.0f,     1.0f, 1.0f, // Bottom-right vertex
        1.0f,  1.0f, 0.0f,     1.0f, 0.0f  // Top-right vertex
    };

    // Create and bind a vertex array object (VAO)
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create a vertex buffer object (VBO) and copy the vertices data to it
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Create an element buffer object (EBO)
    GLuint EBO;
    glGenBuffers(1, &EBO);

    // Draw the quad
    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hudTex);
    glUniform1i(glGetUniformLocation(hudShader.ID, "textureSampler"), 0);

    // Draw the quad using GL_TRIANGLE_FAN since you have 4 vertices
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Unbind the VAO, VBO, EBO, and texture
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Delete VAO, VBO, and EBO
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

static void HandleInput(Params* params) {
    if (params->wDown)
    {
        params->objPos.x += 25.f * params->dt;
    }
    if (params->sDown)
    {
        params->objPos.x -=  25.f * params->dt;
    }
    if (params->aDown)
    {
        params->objPos.z += 14.f * params->dt;
    }
    if (params->dDown)
    {
        params->objPos.z -= 25.f * params->dt;
    }

    //Valley
    if (params->modelRotating) {
        params->modelRot += 50 * params->dt;
    }
    if (params->treeWhiteUpPress) {
        params->treeWhite += 5 * params->dt;
        params->treeWhite = glm::clamp(params->treeWhite, 0.f, 3.f);
    }
    if (params->treeWhiteDownPress) {
        params->treeWhite -= 5 * params->dt;
        params->treeWhite = glm::clamp(params->treeWhite, 0.f, 3.f);
    }

    //Sun scale
    if (!params->sunScaleMaxReached) {
        params->sunScale += 0.4 * params->dt;
    }
    else
    {
        params->sunScale -= 0.4 * params->dt;
    }

    if (params->sunScale >= 2.5) {
        params->sunScaleMaxReached = true;
    }
    if (params->sunScale <= 1.5) {
        params->sunScaleMaxReached = false;
    }

    //Sun color
    if (!params->sunColorMaxReached) {
        params->sunG += 0.2 * params->dt;
    }
    else
    {
        params->sunG -= 0.2 * params->dt;
    }

    if (params->sunG >= 1) {
        params->sunColorMaxReached = true;
    }
    if (params->sunG <= 0.45) {
        params->sunColorMaxReached = false;
    }
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    Params* params = (Params*)glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        params->wDown = true;
    }
    if (key == GLFW_KEY_W && action == GLFW_RELEASE)
    {
        params->wDown = false;
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        params->sDown = true;
    }
    if (key == GLFW_KEY_S && action == GLFW_RELEASE)
    {
        params->sDown = false;
    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        params->aDown = true;
    }
    if (key == GLFW_KEY_A && action == GLFW_RELEASE)
    {
        params->aDown = false;
    }

    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        params->dDown = true;
    }
    if (key == GLFW_KEY_D && action == GLFW_RELEASE)
    {
        params->dDown = false;
    }

    //Valley 
    if (key == GLFW_KEY_Q) {
        if (action == GLFW_PRESS) {
            params->modelRotating = true;
        }
        else if (action == GLFW_RELEASE) {
            params->modelRotating = false;
        }
    }
    if (key == GLFW_KEY_B) {
        if (action == GLFW_PRESS) {
            params->isDay = true;
        }
    }
    if (key == GLFW_KEY_N) {
        if (action == GLFW_PRESS) {
            params->isDay = false;
        }
    }
    if (key == GLFW_KEY_UP) {
        if (action == GLFW_PRESS) {
            params->treeWhiteUpPress = true;
        }
        else if (action == GLFW_RELEASE) {
            params->treeWhiteUpPress = false;
        }
    }
    if (key == GLFW_KEY_DOWN) {
        if (action == GLFW_PRESS) {
            params->treeWhiteDownPress = true;
        }
        else if (action == GLFW_RELEASE) {
            params->treeWhiteDownPress = false;
        }
    }
    if (key == GLFW_KEY_KP_ADD) {
        if (action == GLFW_PRESS) {
            params->appleNumber += 1;
            params->appleNumber = glm::clamp(params->appleNumber, 0, 4);
        }
    }
    if (key == GLFW_KEY_KP_SUBTRACT) {
        if (action == GLFW_PRESS) {
            params->appleNumber -= 1;
            params->appleNumber = glm::clamp(params->appleNumber, 0, 4);
        }
    }
    if (key == GLFW_KEY_1) {
        if (action == GLFW_PRESS) {
            params->appState = 1;
        }
    }
    if (key == GLFW_KEY_2) {
        if (action == GLFW_PRESS) {
            params->appState = 2;
        }
    }
    if (key == GLFW_KEY_3) {
        if (action == GLFW_PRESS) {
            params->appState = 3;
        }
    }
    if (key == GLFW_KEY_4) {
        if (action == GLFW_PRESS) {
            params->appState = 4;
        }
    }
    if (key == GLFW_KEY_C) {
        if (action == GLFW_PRESS) {
            params->isoAngle += 45;
        }
    }
}

static std::vector<float> generateSphereVertices(int slices, int stacks, float radius) {
    std::vector<float> sphereVertices;

    for (int i = 0; i <= stacks; ++i) {
        double lat0 = M_PI * (-0.5 + (double)(i) / stacks);
        double z0 = radius * sin(lat0);
        double zr0 = radius * cos(lat0);

        double lat1 = M_PI * (-0.5 + (double)(i + 1) / stacks);
        double z1 = radius * sin(lat1);
        double zr1 = radius * cos(lat1);

        for (int j = 0; j <= slices; ++j) {
            double lng = 2 * M_PI * (double)j / slices;
            double x0 = cos(lng);
            double y0 = sin(lng);

            lng = 2 * M_PI * (double)(j + 1) / slices;
            double x1 = cos(lng);
            double y1 = sin(lng);

            // Triangle 1
            sphereVertices.push_back(radius * x0 * zr0);  // X
            sphereVertices.push_back(radius * y0 * zr0);  // Y
            sphereVertices.push_back(z0);                // Z

            sphereVertices.push_back(x0 * zr0 / radius);  // NX
            sphereVertices.push_back(y0 * zr0 / radius);  // NY
            sphereVertices.push_back(z0 / radius);       // NZ

            sphereVertices.push_back((float)j / slices);  // U
            sphereVertices.push_back((float)i / stacks);  // V

            // Triangle 2
            sphereVertices.push_back(radius * x1 * zr0);  // X
            sphereVertices.push_back(radius * y1 * zr0);  // Y
            sphereVertices.push_back(z0);                // Z

            sphereVertices.push_back(x1 * zr0 / radius);  // NX
            sphereVertices.push_back(y1 * zr0 / radius);  // NY
            sphereVertices.push_back(z0 / radius);       // NZ

            sphereVertices.push_back((float)(j + 1) / slices);  // U
            sphereVertices.push_back((float)i / stacks);      // V

            // Triangle 3
            sphereVertices.push_back(radius * x0 * zr1);  // X
            sphereVertices.push_back(radius * y0 * zr1);  // Y
            sphereVertices.push_back(z1);                // Z

            sphereVertices.push_back(x0 * zr1 / radius);  // NX
            sphereVertices.push_back(y0 * zr1 / radius);  // NY
            sphereVertices.push_back(z1 / radius);       // NZ

            sphereVertices.push_back((float)j / slices);      // U
            sphereVertices.push_back((float)(i + 1) / stacks);  // V

            // Triangle 4
            sphereVertices.push_back(radius * x1 * zr1);  // X
            sphereVertices.push_back(radius * y1 * zr1);  // Y
            sphereVertices.push_back(z1);                // Z

            sphereVertices.push_back(x1 * zr1 / radius);  // NX
            sphereVertices.push_back(y1 * zr1 / radius);  // NY
            sphereVertices.push_back(z1 / radius);       // NZ

            sphereVertices.push_back((float)(j + 1) / slices);      // U
            sphereVertices.push_back((float)(i + 1) / stacks);  // V
        }
    }
    return sphereVertices;
}

int main()
{
    HWND console = GetConsoleWindow();
    SetWindowPos(console, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    if(!glfwInit())
    {
        std::cout << "GLFW fail!\n" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(wWidth, wHeight, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Window fail!\n" << std::endl;
        glfwTerminate();
        return -2;
    }

    glfwSetWindowPos(window, 0, 40);
    glfwMakeContextCurrent(window);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() !=GLEW_OK)
    {
        std::cout << "GLEW fail! :(\n" << std::endl;
        return -3;
    }

    //START
    std::vector<float> cubeVertices = {
        // X     Y     Z     NX    NY    NZ    U     V    FRONT SIDE
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // L D
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // R D
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // L U
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // R D
        0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // R U
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // L U
        // LEFT SIDE
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // R U
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
        // RIGHT SIDE
        0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
        0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // R U
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
        // BOTTOM SIDE
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // L U
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // R D
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // R U
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // L U
        // TOP SIDE
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // L U
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // R U
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // L U
        // BACK SIDE
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // L U
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // R U
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // L U
    };
    GameObject* simpleCube = new GameObject(cubeVertices);

    const int slices = 30;
    const int stacks = 30;
    const float radius = 1.0f;

    std::vector<float> sphereVertices = generateSphereVertices(slices, stacks, radius);
    GameObject* simpleSphere = new GameObject(sphereVertices);

    Model appleObj("res/apple2_UV.OBJ");
    Model archerObj("res/Archer.obj");

    Shader currentShader;
    Shader phongShader("phong.vert", "phong.frag");
    Shader goroShader("goro.vert", "goro.frag");
    Shader hudShader("hud.vert", "hud.frag");
    Shader twoD("twoD.vert", "twoD.frag");
    Shader unlitShader("unlit.vert", "unlit.frag");

    currentShader = unlitShader;

    currentShader.use();

    glm::mat4 view;
    glm::mat4 projectionP;

    unsigned hudTex = Model::textureFromFile("res/hudTex.png");
    unsigned grassD = Model::textureFromFile("res/grassD.png");
    unsigned grassS = Model::textureFromFile("res/grassS.png");

    glm::mat4 model2 = glm::mat4(1.0f);
    glm::mat4 m(1.0f);
    float currentRot = 0;
    float FrameStartTime = 0;
    float FrameEndTime = 0;
    glm::vec3 rotPoint = glm::vec3(25.0, 40.0, 25.0);

    Params params;
    glfwSetWindowUserPointer(window, &params);

    glClearColor(1, 0.72, 0.33, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    
    while (!glfwWindowShouldClose(window))
    {
        FrameStartTime = glfwGetTime();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //APP State managment
        if (params.appState == 1) {
            currentShader = goroShader;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else if(params.appState == 2)
        {
            currentShader = unlitShader;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else if (params.appState == 3)
        {
            currentShader = goroShader;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else
        {
            currentShader = phongShader;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        currentShader.setInt("uMaterial.Kd", 0);
        currentShader.setInt("uMaterial.Ks", 1);
        currentShader.setFloat("uMaterial.Shininess", 0.5 * 128);

        currentShader.setVec3("uSpotlights[1].Position", glm::vec3(-99999));
        currentShader.setVec3("uSpotlights[1].Direction", 0.0, 0.0, -1.0);
        currentShader.setVec3("uSpotlights[1].Ka", 0.0, 0.0, 0.0);
        currentShader.setVec3("uSpotlights[1].Kd", glm::vec3(1.0f, 1.0f, 1.0f));
        currentShader.setVec3("uSpotlights[1].Ks", glm::vec3(1.0));
        currentShader.setFloat("uSpotlights[1].InnerCutOff", glm::cos(glm::radians(15.0f)));
        currentShader.setFloat("uSpotlights[1].OuterCutOff", glm::cos(glm::radians(20.0f)));
        currentShader.setFloat("uSpotlights[1].Kc", 1.0);
        currentShader.setFloat("uSpotlights[1].Kl", 0.092f);
        currentShader.setFloat("uSpotlights[1].Kq", 0.032f);
        //-----------------------------------------------------------------------------
        //Loop
        currentShader.use();
        HandleInput(&params);

        //Camera 
        float angle = glm::radians(params.isoAngle);

        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec4 rotatedPos = rotationMatrix * glm::vec4(rotPoint+ params.objPos, 1.0f);
        glm::vec3 rotatedObjPos = glm::vec3(rotatedPos);

        view = glm::lookAt(rotatedObjPos, glm::vec3(0), params.cameraUp);

        currentShader.setMat4("uView", view);
        currentShader.setVec3("uViewPos", rotatedObjPos);

        float orthoScale = 1.55;
        projectionP = glm::ortho(-16.f* orthoScale, 16.f * orthoScale, -9.f * orthoScale, 9.f * orthoScale, -200.f, 200.f);
        currentShader.setMat4("uProjection", projectionP);

        //SCENE
        //------------------------------------------------------------------------------------------------------------
        //Ground
        float treeThick = 0.3;
        float treeHegiht = 3;

        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
        m = glm::scale(m, glm::vec3(10.0, 1.0, 10.0));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, grassD, grassS);

        //Sun   
        if (params.isDay) {
            glClearColor(0.35, 0.59, 1, 1.0);
            m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 15, 0.0));
            m = glm::rotate(m, glm::radians(30.f), glm::vec3(1.0, 0.0, 1.0));
            m = glm::scale(m, glm::vec3(params.sunScale));
            currentShader.setMat4("uModel", m);
            simpleCube->Render(&currentShader, 0.92, params.sunG, 0.04);
            //simpleSphere->Render(&currentShader, 0.92, params.sunG, 0.04, true);

            currentShader.setVec3("uDirLight.Position", 0.0, 20, 0.0);
            currentShader.setVec3("uDirLight.Direction", 0.0, -1, 0.0);
            currentShader.setVec3("uDirLight.Ka", glm::vec3(0.3));
            currentShader.setVec3("uDirLight.Kd", glm::vec3(0.5));
            currentShader.setVec3("uDirLight.Ks", glm::vec3(1.0));
        }
        else
        {
            glClearColor(0.05, 0.13, 0.25, 1.0);
            m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 15, 0.0));
            m = glm::rotate(m, glm::radians(30.f), glm::vec3(1.0, 0.0, 1.0));
            m = glm::scale(m, glm::vec3(2.0));
            currentShader.setMat4("uModel", m);
            simpleCube->Render(&currentShader, 0.7, 0.7, 0.64);

            //Stars
            m = glm::translate(glm::mat4(1.0), glm::vec3(2.0, 15, -2.0));
            m = glm::scale(m, glm::vec3(0.2));
            currentShader.setMat4("uModel", m);
            simpleCube->Render(&currentShader, 1, 1, 0);

            m = glm::translate(glm::mat4(1.0), glm::vec3(3.0, 15.5, 2.0));
            m = glm::scale(m, glm::vec3(0.2));
            currentShader.setMat4("uModel", m);
            simpleCube->Render(&currentShader, 1, 1, 0);

            m = glm::translate(glm::mat4(1.0), glm::vec3(-3.0, 16, 0.5));
            m = glm::scale(m, glm::vec3(0.3));
            currentShader.setMat4("uModel", m);
            simpleCube->Render(&currentShader, 1, 1, 0);

            currentShader.setVec3("uDirLight.Position", 0.0, 20, 0.0);
            currentShader.setVec3("uDirLight.Direction", 0.5, -1, 0.5);
            currentShader.setVec3("uDirLight.Ka", glm::vec3(0.51, 0.58, 0.78)/6.f);
            currentShader.setVec3("uDirLight.Kd", glm::vec3(0.51, 0.58, 0.78)/4.f);
            currentShader.setVec3("uDirLight.Ks", glm::vec3(0.51, 0.58, 0.78));
        }
        //Trunk1
        m = glm::translate(glm::mat4(1.0), glm::vec3(4.0, treeHegiht/2+0.5, 1.0));
        m = glm::scale(m, glm::vec3(treeThick, treeHegiht, treeThick));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, 0.26, 0.19, 0.01);

        //Trunk2
        m = glm::translate(glm::mat4(1.0), glm::vec3(3.0, treeHegiht / 2 + 0.5, -2.0));
        m = glm::scale(m, glm::vec3(treeThick, treeHegiht, treeThick));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, 0.26, 0.19, 0.01);

        //Trunk3
        m = glm::translate(glm::mat4(1.0), glm::vec3(1.0, treeHegiht / 2 + 0.5, 3.0));
        m = glm::scale(m, glm::vec3(treeThick, treeHegiht, treeThick));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, 0.26, 0.19, 0.01);

        //AppleTrunk
        float appleTreeThick = 0.5;
        float appleTreeHeight = 4.0;

        m = glm::translate(glm::mat4(1.0), glm::vec3(-2.5, appleTreeHeight/2+0.5, 0.0));
        m = glm::scale(m, glm::vec3(appleTreeThick, appleTreeHeight, appleTreeThick));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, 0.41, 0.31, 0.05);

        //White
        float scale = params.treeWhite;
        float yoffscet = (scale) / 2;

        m = glm::translate(glm::mat4(1.0), glm::vec3(-2.5, yoffscet + 0.5, 0.0));
        m = glm::scale(m, glm::vec3(appleTreeThick+0.001, scale, appleTreeThick+0.001));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, 0.86, 0.83, 0.77);

        //Archer
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.0, 0.5, -1.0));
        m = glm::rotate(m, glm::radians(params.modelRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.2));
        currentShader.setMat4("uModel", m);
        archerObj.Draw(currentShader);

        glm::vec3 positionVector = glm::vec3(m[3]);
        // Extract forward vector from rotation submatrix
        glm::vec3 forwardVector = glm::vec3(m[2]);
        positionVector.y += 1;
        forwardVector.y += 0.03;

        currentShader.setVec3("uSpotlights[0].Position", positionVector);
        currentShader.setVec3("uSpotlights[0].Direction", forwardVector);
        currentShader.setVec3("uSpotlights[0].Ka", 0.0, 0.0, 0.0);
        currentShader.setVec3("uSpotlights[0].Kd", glm::vec3(2.0f, 2.0f, 2.0f));
        currentShader.setVec3("uSpotlights[0].Ks", glm::vec3(1.0));
        currentShader.setFloat("uSpotlights[0].InnerCutOff", glm::cos(glm::radians(25.0f)));
        currentShader.setFloat("uSpotlights[0].OuterCutOff", glm::cos(glm::radians(35.0f)));
        currentShader.setFloat("uSpotlights[0].Kc", 1.0);
        currentShader.setFloat("uSpotlights[0].Kl", 0.092f);
        currentShader.setFloat("uSpotlights[0].Kq", 0.032f);

        //Apples
        float kc = 1.5;
        float kl = 1.0;
        float kq = 0.9;
        glm::vec3 zeroV = glm::vec3(0);
        if (params.appleNumber >= 1) {
            glm::vec3 pos = glm::vec3(-2.5 + appleTreeThick * 2.4, appleTreeHeight + 0.5 + appleTreeThick * 6 / 2, appleTreeThick * 2.4);
            m = glm::translate(glm::mat4(1.0), pos);
            m = glm::rotate(m, glm::radians(90.f), glm::vec3(1.0, 0.0, 0.0));
            m = glm::scale(m, glm::vec3(0.02));
            currentShader.setMat4("uModel", m);
            appleObj.Draw(currentShader);

            currentShader.setVec3("uPointLights[0].Position", pos);
            currentShader.setVec3("uPointLights[0].Ka", glm::vec3(0.22, 0.11, 0.66)/2.f);
            currentShader.setVec3("uPointLights[0].Kd", glm::vec3(0.22, 0.11, 0.66)*10.f);
            currentShader.setVec3("uPointLights[0].Ks", glm::vec3(0.22, 0.11, 0.66));
            currentShader.setFloat("uPointLights[0].Kc", kc);
            currentShader.setFloat("uPointLights[0].Kl", kl);
            currentShader.setFloat("uPointLights[0].Kq", kq);
        }
        else {
            currentShader.setVec3("uPointLights[0].Position", zeroV);
            currentShader.setVec3("uPointLights[0].Ka", zeroV);
            currentShader.setVec3("uPointLights[0].Kd", zeroV);
            currentShader.setVec3("uPointLights[0].Ks", zeroV);
            currentShader.setFloat("uPointLights[0].Kc", kc);
            currentShader.setFloat("uPointLights[0].Kl", kl);
            currentShader.setFloat("uPointLights[0].Kq", kq);
        }

        if (params.appleNumber >= 2) {
            glm::vec3 pos = glm::vec3(-2.5 + appleTreeThick * 2.4, appleTreeHeight + 0.5 + appleTreeThick * 6 / 2, -appleTreeThick * 2.4);
            m = glm::translate(glm::mat4(1.0), pos);
            m = glm::rotate(m, glm::radians(90.f), glm::vec3(1.0, 0.0, 0.0));
            m = glm::scale(m, glm::vec3(0.02));
            currentShader.setMat4("uModel", m);
            appleObj.Draw(currentShader);

            currentShader.setVec3("uPointLights[1].Position", pos);
            currentShader.setVec3("uPointLights[1].Ka", glm::vec3(0.22, 0.11, 0.66) / 2.f);
            currentShader.setVec3("uPointLights[1].Kd", glm::vec3(0.22, 0.11, 0.66) * 10.f);
            currentShader.setVec3("uPointLights[1].Ks", glm::vec3(0.22, 0.11, 0.66));
            currentShader.setFloat("uPointLights[1].Kc", kc);
            currentShader.setFloat("uPointLights[1].Kl", kl);
            currentShader.setFloat("uPointLights[1].Kq", kq);
        }
        else {
            currentShader.setVec3("uPointLights[1].Position", zeroV);
            currentShader.setVec3("uPointLights[1].Ka", zeroV);
            currentShader.setVec3("uPointLights[1].Kd", zeroV);
            currentShader.setVec3("uPointLights[1].Ks", zeroV);
            currentShader.setFloat("uPointLights[1].Kc", kl);
            currentShader.setFloat("uPointLights[1].Kl", kc);
            currentShader.setFloat("uPointLights[1].Kq", kq);
        }

        if (params.appleNumber >= 3) {
            glm::vec3 pos = glm::vec3(-2.5 + -appleTreeThick * 2.4, appleTreeHeight + 0.5 + appleTreeThick * 6 / 2, -appleTreeThick * 2.4);
            m = glm::translate(glm::mat4(1.0), pos);
            m = glm::rotate(m, glm::radians(90.f), glm::vec3(1.0, 0.0, 0.0));
            m = glm::scale(m, glm::vec3(0.02));
            currentShader.setMat4("uModel", m);
            appleObj.Draw(currentShader);

            currentShader.setVec3("uPointLights[2].Position", pos);
            currentShader.setVec3("uPointLights[2].Ka", glm::vec3(0.11, 0.66, 0.43) / 2.f);
            currentShader.setVec3("uPointLights[2].Kd", glm::vec3(0.11, 0.66, 0.43) * 10.f);
            currentShader.setVec3("uPointLights[2].Ks", glm::vec3(0.11, 0.66, 0.43));
            currentShader.setFloat("uPointLights[2].Kc", kc);
            currentShader.setFloat("uPointLights[2].Kl", kl);
            currentShader.setFloat("uPointLights[2].Kq", kq);
        }
        else {
            currentShader.setVec3("uPointLights[2].Position", zeroV);
            currentShader.setVec3("uPointLights[2].Ka", zeroV);
            currentShader.setVec3("uPointLights[2].Kd", zeroV);
            currentShader.setVec3("uPointLights[2].Ks", zeroV);
            currentShader.setFloat("uPointLights[2].Kc", kc);
            currentShader.setFloat("uPointLights[2].Kl", kl);
            currentShader.setFloat("uPointLights[2].Kq", kq);
        }

        if (params.appleNumber >= 4) {
            glm::vec3 pos = glm::vec3(-2.5 - appleTreeThick * 2.4, appleTreeHeight + 0.5 + appleTreeThick * 6 / 2, appleTreeThick * 2.4);
            m = glm::translate(glm::mat4(1.0), pos);
            m = glm::rotate(m, glm::radians(90.f), glm::vec3(1.0, 0.0, 0.0));
            m = glm::scale(m, glm::vec3(0.02));
            currentShader.setMat4("uModel", m);
            appleObj.Draw(currentShader);

            currentShader.setVec3("uPointLights[3].Position", pos);
            currentShader.setVec3("uPointLights[3].Ka", glm::vec3(0.66, 0.11, 0.25) / 2.f);
            currentShader.setVec3("uPointLights[3].Kd", glm::vec3(0.66, 0.11, 0.25) * 10.f);
            currentShader.setVec3("uPointLights[3].Ks", glm::vec3(0.66, 0.11, 0.25));
            currentShader.setFloat("uPointLights[3].Kc", kc);
            currentShader.setFloat("uPointLights[3].Kl", kl);
            currentShader.setFloat("uPointLights[3].Kq", kq);
        }
        else {
            currentShader.setVec3("uPointLights[3].Position", zeroV);
            currentShader.setVec3("uPointLights[3].Ka", zeroV);
            currentShader.setVec3("uPointLights[3].Kd", zeroV);
            currentShader.setVec3("uPointLights[3].Ks", zeroV);
            currentShader.setFloat("uPointLights[3].Kc", kc);
            currentShader.setFloat("uPointLights[3].Kl", kl);
            currentShader.setFloat("uPointLights[3].Kq", kq);
        }


        //Leaves1
        currentShader.setBool("uTransp", true);
        currentShader.setFloat("uAlpha", 0.7f);

        m = glm::translate(glm::mat4(1.0), glm::vec3(4.0, treeHegiht + 0.5 + treeThick*6/2, 1.0));
        m = glm::rotate(m, glm::radians(32.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(treeThick*6, treeThick * 6, treeThick*6));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, 0, 1, 0);

        //Leaves2
        m = glm::translate(glm::mat4(1.0), glm::vec3(3.0, treeHegiht + 0.5 + treeThick * 6 / 2, -2.0));
        m = glm::rotate(m, glm::radians(18.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(treeThick * 6, treeThick * 6, treeThick * 6));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, 0, 1, 0);

        //Leaves3
        m = glm::translate(glm::mat4(1.0), glm::vec3(1.0, treeHegiht + 0.5 + treeThick * 6 / 2, 3.0));
        m = glm::rotate(m, glm::radians(76.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(treeThick * 6, treeThick * 6, treeThick * 6));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, 0, 1, 0);

        //LeavesApple
        m = glm::translate(glm::mat4(1.0), glm::vec3(-2.5, appleTreeHeight + 0.5 + appleTreeThick * 6 / 2, 0.0));
        m = glm::rotate(m, glm::radians(45.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(appleTreeThick * 6, appleTreeThick * 6, appleTreeThick * 6));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, 0, 1, 0);
        currentShader.setBool("uTransp", false);

        //Clouds
        params.cloudsZoffset = fmod(params.cloudsZoffset+params.dt, 5.0f);

        currentShader.setBool("uTransp", true);
        currentShader.setFloat("uAlpha", 0.5f);

        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 9, -1.0+params.cloudsZoffset));
        m = glm::scale(m, glm::vec3(1.0, 0.8, 2.0));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, 1, 1, 1);

        m = glm::translate(glm::mat4(1.0), glm::vec3(3.0, 10, 4.0 + params.cloudsZoffset));
        m = glm::scale(m, glm::vec3(0.8, 0.8, 1.7));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, 1, 1, 1);

        m = glm::translate(glm::mat4(1.0), glm::vec3(-4.0, 10.5, 2.0 + params.cloudsZoffset));
        m = glm::scale(m, glm::vec3(1.0, 0.6, 3.0));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, 1, 1, 1);

        m = glm::translate(glm::mat4(1.0), glm::vec3(2.0, 11, -3.0 + params.cloudsZoffset));
        m = glm::scale(m, glm::vec3(2.0, 0.7, 1.0));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, 1, 1, 1);

        currentShader.setBool("uTransp", false);
        //------------------------------------------------------------------------------------------------------------

        //HUD
        DrawHud(hudShader, hudTex);
        currentShader.use();

        glfwSwapBuffers(window);
        glfwPollEvents();

        FrameEndTime = glfwGetTime();
        params.dt = FrameEndTime - FrameStartTime;
    }

    glfwTerminate();
    return 0;
}


