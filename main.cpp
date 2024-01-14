//Opis: Primjer ucitavanja modela upotrebom ASSIMP biblioteke
//Preuzeto sa learnOpenGL

#define _CRT_SECURE_NO_WARNINGS

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

const unsigned int wWidth = 1920;
const unsigned int wHeight = 1080;

bool firstMouse = true;
double lastX;
double lastY;

struct Params {
    float dt = 0;
    bool isFps = true;

    bool isCurosIn = true;
    double xPosC = 0.0;
    double yPosC = 0.0;

    glm::vec3 cameraFront = glm::vec3(0.0, 0.0, 1.0);
    glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

    glm::vec3 position = glm::vec3(0.0, 0.0, -1.0);
    glm::vec3 objPos = glm::vec3(0.0, 0.0, 0.0);

    double camYaw = 90;
    double camPitch = 0;

    bool wDown = false;
    bool sDown = false;
    bool aDown = false;
    bool dDown = false;

    bool spaceDown = false;
    bool shiftDown = false;

    //House
    float dogRot = 0;
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
}

static void HandleInput(Params* params) {
    if (params->wDown)
    {
        if (params->isFps)
            params->position += 7.2f * params->cameraFront * params->dt;
        else
            params->objPos.z += 0.5f * params->dt;
    }
    if (params->sDown)
    {
        if (params->isFps)
            params->position -= 7.2f * params->cameraFront * params->dt;
        else
            params->objPos.z -= 0.5f * params->dt;
    }
    if (params->aDown)
    {

        glm::vec3 strafe = glm::cross(params->cameraFront, params->cameraUp);
        if (params->isFps)
            params->position -= 7.2f * strafe * params->dt;
        else
            params->objPos.x += 0.5f * params->dt;
    }
    if (params->dDown)
    {
        glm::vec3 strafe = glm::cross(params->cameraFront, params->cameraUp);
        if (params->isFps)
            params->position += 7.2f * strafe * params->dt;
        else
            params->objPos.x -= 0.5f * params->dt;
    }
    if (params->spaceDown)
    {
        if (params->isFps)
            params->position.y += 4.2 * params->dt;
        else
            params->objPos.y += 0.5f * params->dt;
    }
    if (params->shiftDown)
    {
        if (params->isFps)
            params->position.y -= 4.1 * params->dt;
        else
            params->objPos.y -= 0.5f * params->dt;
    }

    //House
    params->dogRot += 50 * params->dt;
    if (params->dogRot > 360) {
        params->dogRot -= 360;
    }
}

static void CursosPosCallback(GLFWwindow* window, double xPos, double yPos) {
    Params* params = (Params*)glfwGetWindowUserPointer(window);

    if (params->isCurosIn) {
        params->xPosC = xPos;
        params->yPosC = yPos;
    }

    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    double xoffset = xPos - lastX;
    double yoffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    float sensitivity = 0.3f;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    params->camYaw += xoffset;
    params->camPitch += yoffset;

    if (params->camPitch > 89.0) {
        params->camPitch = 89.0;
    }
    else if (params->camPitch < -89.0) {
        params->camPitch = -89.0;
    }

    glm::vec3 front;
    front.x = cos(glm::radians(params->camYaw)) * cos(glm::radians(params->camPitch));
    front.y = sin(glm::radians(params->camPitch));
    front.z = sin(glm::radians(params->camYaw)) * cos(glm::radians(params->camPitch));

    params->cameraFront = glm::normalize(front);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    Params* params = (Params*)glfwGetWindowUserPointer(window);
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        std::cout << "glm::vec3(" << params->objPos.x << "," << params->objPos.y << "," << params->objPos.z << ")" << std::endl;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        params->isFps = !params->isFps;
    }

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

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        params->spaceDown = true;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
    {
        params->spaceDown = false;
    }

    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
    {
        params->shiftDown = true;
    }
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
    {
        params->shiftDown = false;
    }
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

    glfwSetWindowPos(window, 300, 120);
    glfwMakeContextCurrent(window);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursosPosCallback);
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

    float tiling = 5;
    std::vector<float> cubeVertices2 = {
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
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f* tiling, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f * tiling, // L U
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f * tiling, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f * tiling, 1.0f * tiling, // R U
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f * tiling, // L U
        // BACK SIDE
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // L U
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // R U
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // L U
    };
    GameObject* simpleCube2 = new GameObject(cubeVertices2);

    Model lija("res/low-poly-fox.obj");
    Model treeObj("res/Tree2.obj");
    Model maxwell("res/maxwell.obj");
    Model dogModel("res/Mesh_Beagle.obj");

    Shader phongShader("phong.vert", "phong.frag");
    Shader hudShader("hud.vert", "hud.frag");
    Shader twoD("twoD.vert", "twoD.frag");

    phongShader.use();

    glm::mat4 view;
    glm::mat4 projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
    phongShader.setMat4("uProjection", projectionP);

    phongShader.setVec3("uDirLight.Position", 0.0, 5, 0.0);
    phongShader.setVec3("uDirLight.Direction", 0.4, -1, 0.1);
    phongShader.setVec3("uDirLight.Ka", glm::vec3(0.3));
    phongShader.setVec3("uDirLight.Kd", glm::vec3(0.6));
    phongShader.setVec3("uDirLight.Ks", glm::vec3(1.0, 1.0, 1.0));

    phongShader.setVec3("uSpotlights[0].Position", glm::vec3(-99999));
    phongShader.setVec3("uSpotlights[0].Direction", 0.0, -1.0, 0.0);
    phongShader.setVec3("uSpotlights[0].Ka", 0.0, 0.0, 0.0);
    phongShader.setVec3("uSpotlights[0].Kd", glm::vec3(3.0f, 3.0f, 3.0f));
    phongShader.setVec3("uSpotlights[0].Ks", glm::vec3(1.0));
    phongShader.setFloat("uSpotlights[0].InnerCutOff", glm::cos(glm::radians(10.0f)));
    phongShader.setFloat("uSpotlights[0].OuterCutOff", glm::cos(glm::radians(15.0f)));
    phongShader.setFloat("uSpotlights[0].Kc", 1.0);
    phongShader.setFloat("uSpotlights[0].Kl", 0.092f);
    phongShader.setFloat("uSpotlights[0].Kq", 0.032f);

    phongShader.setVec3("uSpotlights[1].Position", 0.0, 0.0, 6.0);
    phongShader.setVec3("uSpotlights[1].Direction", 0.0, 0.0, -1.0);
    phongShader.setVec3("uSpotlights[1].Ka", 0.0, 0.0, 0.0);
    phongShader.setVec3("uSpotlights[1].Kd", glm::vec3(1.0f, 1.0f, 1.0f));
    phongShader.setVec3("uSpotlights[1].Ks", glm::vec3(1.0));
    phongShader.setFloat("uSpotlights[1].InnerCutOff", glm::cos(glm::radians(15.0f)));
    phongShader.setFloat("uSpotlights[1].OuterCutOff", glm::cos(glm::radians(20.0f)));
    phongShader.setFloat("uSpotlights[1].Kc", 1.0);
    phongShader.setFloat("uSpotlights[1].Kl", 0.092f);
    phongShader.setFloat("uSpotlights[1].Kq", 0.032f);

    phongShader.setVec3("uPointLights[0].Position", glm::vec3(-99999));
    phongShader.setVec3("uPointLights[0].Ka", glm::vec3(230.0 / 255 / 0.1, 92.0 / 255 / 0.1, 0.0f));
    phongShader.setVec3("uPointLights[0].Kd", glm::vec3(230.0 / 255 / 50, 92.0 / 255 / 50, 0.0f));
    phongShader.setVec3("uPointLights[0].Ks", glm::vec3(1.0f));
    phongShader.setFloat("uPointLights[0].Kc", 1.5f);
    phongShader.setFloat("uPointLights[0].Kl", 1.0f);
    phongShader.setFloat("uPointLights[0].Kq", 0.272f);

    unsigned hudTex = Model::textureFromFile("res/hudTex.png");
    unsigned kockaDif = Model::textureFromFile("res/container_diffuse.png");
    unsigned kockaSpec = Model::textureFromFile("res/container_specular.png");
    unsigned tapeteDif = Model::textureFromFile("res/tapete.png");
    unsigned podDif = Model::textureFromFile("res/podDif.png");
    unsigned podSpec = Model::textureFromFile("res/podSpec.png");
    unsigned grassDif = Model::textureFromFile("res/grassDif.png");
    unsigned grassSpec = Model::textureFromFile("res/grassSpec.png");

    phongShader.setInt("uMaterial.Kd", 0);
    phongShader.setInt("uMaterial.Ks", 1);
    phongShader.setFloat("uMaterial.Shininess", 0.5 * 128);

    glm::mat4 model2 = glm::mat4(1.0f);
    glm::mat4 m(1.0f);
    float currentRot = 0;
    float FrameStartTime = 0;
    float FrameEndTime = 0;

    Params params;
    glfwSetWindowUserPointer(window, &params);

    glClearColor(0.2, 0.2, 0.6, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    while (!glfwWindowShouldClose(window))
    {
        FrameStartTime = glfwGetTime();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Loop
        phongShader.use();
        HandleInput(&params);

        //Camera
        view = glm::lookAt(params.position,params.position + params.cameraFront,params.cameraUp);

        phongShader.setMat4("uView", view);
        phongShader.setVec3("uViewPos", params.position);

        //SCENE
        //------------------------------------------------------------------------------------------------------------
        //Ground
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
        //m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(30.0, 1.0, 30.0));
        phongShader.setMat4("uModel", m);
        simpleCube2->Render(&phongShader, grassDif, grassSpec);

        //Pod
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.51, 0.0));
        m = glm::scale(m, glm::vec3(6.0, 0.1, 6.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, podDif, podSpec);

        //Zadnj zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 2.5, 3.0));
        m = glm::scale(m, glm::vec3(6.0, 4.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Sa leve strane zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(3.0, 2.5, 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(6.2, 4.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Sa desne strane zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(-3.0, 2.5, 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(6.2, 4.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Zid sa vratima
        m = glm::translate(glm::mat4(1.0), glm::vec3(1.5, 2.5, -3.0));
        m = glm::scale(m, glm::vec3(3.0, 4.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Zid ispod prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.5, 1.0, -3.0));
        m = glm::scale(m, glm::vec3(3.0, 1.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Zid iznad prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.5, 4.0, -3.0));
        m = glm::scale(m, glm::vec3(3.0, 1.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Zid desno od prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-2.5, 2.5, -3.0));
        m = glm::scale(m, glm::vec3(1.0, 2.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //SPRAT 2
        float offset = 4;

        //Pod
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.51+ offset, 0.0));
        m = glm::scale(m, glm::vec3(6.0, 0.1, 6.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, podDif, podSpec);

        //Zadnj zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 2.5+ offset, 3.0));
        m = glm::scale(m, glm::vec3(6.0, 4.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Sa leve strane zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(3.0, 2.5+ offset, 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(6.2, 4.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Sa desne strane zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(-3.0, 2.5+ offset, 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(6.2, 4.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Zid sa vratima
        m = glm::translate(glm::mat4(1.0), glm::vec3(1.5, 2.5+ offset, -3.0));
        m = glm::scale(m, glm::vec3(3.0, 4.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Zid ispod prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.5, 1.0+offset, -3.0));
        m = glm::scale(m, glm::vec3(3.0, 1.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Zid iznad prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.5, 4.0+ offset, -3.0));
        m = glm::scale(m, glm::vec3(3.0, 1.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Zid desno od prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-2.5, 2.5+ offset, -3.0));
        m = glm::scale(m, glm::vec3(1.0, 2.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Pod
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.55 + offset*2, 0.0));
        m = glm::scale(m, glm::vec3(6.2, 0.1, 6.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);


        //UNUTRASNJI ZIDOVI
        float innerOffset = 0.15;

        //Zadnj zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 2.5 + offset, 3.0- innerOffset));
        m = glm::scale(m, glm::vec3(5.99, 4.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Sa leve strane zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(3.0- innerOffset, 2.5 + offset, 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(6.19, 4.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Sa desne strane zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(-3.0+ innerOffset, 2.5 + offset, 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(6.19, 4.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Zid sa vratima
        m = glm::translate(glm::mat4(1.0), glm::vec3(1.5, 2.5 + offset, -3.0+ innerOffset));
        m = glm::scale(m, glm::vec3(2.99, 4.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Zid ispod prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.49, 1.0 + offset, -3.0+ innerOffset));
        m = glm::scale(m, glm::vec3(3.0, 1.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Zid iznad prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.49, 4.0 + offset, -3.0+ innerOffset));
        m = glm::scale(m, glm::vec3(2.99, 1.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Zid desno od prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-2.5, 2.5 + offset, -3.0+ innerOffset));
        m = glm::scale(m, glm::vec3(0.99, 2.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Sprat1Inner

        //Zadnj zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 2.5 , 3.0 - innerOffset));
        m = glm::scale(m, glm::vec3(5.99, 4.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Sa leve strane zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(3.0 - innerOffset, 2.5 , 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(6.19, 4.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Sa desne strane zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(-3.0 + innerOffset, 2.5 , 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(6.19, 4.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Zid sa vratima
        m = glm::translate(glm::mat4(1.0), glm::vec3(1.5, 2.5 , -3.0 + innerOffset));
        m = glm::scale(m, glm::vec3(2.99, 4.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Zid ispod prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.49, 1.0 , -3.0 + innerOffset));
        m = glm::scale(m, glm::vec3(3.0, 1.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Zid iznad prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.49, 4.0 , -3.0 + innerOffset));
        m = glm::scale(m, glm::vec3(2.99, 1.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Zid desno od prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-2.5, 2.5 , -3.0 + innerOffset));
        m = glm::scale(m, glm::vec3(0.99, 2.0, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tapeteDif);

        //Vrata
        m = glm::translate(glm::mat4(1.0), glm::vec3(1.5, 1.5, -3.0-innerOffset/2));
        m = glm::scale(m, glm::vec3(1.2, 2.2, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.46, 0.25, 0.11);

        //Dimnjak
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.5, 0.55 + offset * 2 + 1, 1.5));
        m = glm::scale(m, glm::vec3(1.2, 2.0, 1.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 0, 0);

        //Sunce
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 25.0, 2.0));
        m = glm::rotate(m, glm::radians(22.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(2.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Ograda

        m = glm::translate(glm::mat4(1.0), glm::vec3(-3.25-1.0, 1.0, -6.0));
        m = glm::scale(m, glm::vec3(6.5, 1.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.46, 0.25, 0.11);

        m = glm::translate(glm::mat4(1.0), glm::vec3(3.25+1.0, 1.0, -6.0));
        m = glm::scale(m, glm::vec3(6.5, 1.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.46, 0.25, 0.11);

        //FanceOpen
        m = glm::translate(glm::mat4(1.0), glm::vec3(1.0, 1.0, -6.0));
        m = glm::rotate(m, glm::radians(22.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -glm::vec3(1.0, 1.0, -6.0));

        m = glm::translate(m, glm::vec3(0.0, 1.0, -6.0));
        m = glm::scale(m, glm::vec3(2, 0.9, 0.11));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.36, 0.15, 0.01);

        //Drvo
        m = glm::translate(glm::mat4(1.0), glm::vec3(-3.0, 0.5, -5.0));
        m = glm::scale(m, glm::vec3(0.5));
        phongShader.setMat4("uModel", m);
        treeObj.Draw(phongShader); 

        //Sto za bice
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.0, 0.5+0.4, -2.0));
        m = glm::scale(m, glm::vec3(1.6, 0.8, 1.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 0, 0);

        //Bice
        glDisable(GL_CULL_FACE);
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.0, 0.5 + 0.8, -2.1));
        m = glm::rotate(m, glm::radians(130.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.03));
        phongShader.setMat4("uModel", m);
        maxwell.Draw(phongShader);
        

        //Pas
        glm::vec3 objectPosition(8.5, 1.2, -8.5);
        glm::vec3 targetPosition(0.0, 0.0, 0.0);

        // Calculate the direction from the object position to the target position
        glm::vec3 direction = glm::normalize(targetPosition - objectPosition);

        // Calculate the right vector (assuming up is (0, 1, 0))
        glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0, 1.0, 0.0), direction));

        // Calculate the up vector
        glm::vec3 up = glm::cross(direction, right);

        // Create a rotation matrix using the calculated vectors
        glm::mat4 rotationMatrix(1.0);
        rotationMatrix[0] = glm::vec4(right, 0.0);
        rotationMatrix[1] = glm::vec4(up, 0.0);
        rotationMatrix[2] = glm::vec4(-direction, 0.0);

        m = glm::rotate(glm::mat4(1.0), glm::radians(params.dogRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, glm::vec3(8.5, 1.2, -8.5));
        m = m * rotationMatrix;
        m = glm::rotate(m, glm::radians(-90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.017));
        phongShader.setMat4("uModel", m);
        dogModel.Draw(phongShader);
        glEnable(GL_CULL_FACE);


        
        phongShader.setBool("uTransp", true);
        phongShader.setFloat("uAlpha", 0.5);

        //Dim
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.5, 0.55 + offset * 2 + 3.5, 1.5));
        m = glm::rotate(m, glm::radians(22.f), glm::vec3(1.0, 0.0, 1.0));
        m = glm::scale(m, glm::vec3(0.5, 1.0, 0.5));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 1);

        //Dim
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.5, 0.55 + offset * 2 + 5.5, 1.5));
        m = glm::rotate(m, glm::radians(-22.f), glm::vec3(1.0, 0.0, 1.0));
        m = glm::scale(m, glm::vec3(0.5, 1.4, 0.5));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 1);

        phongShader.setFloat("uAlpha", 0.3);

        //Prozor 1
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.0, 2.5, -3.0));
        m = glm::scale(m, glm::vec3(2.0, 2.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        //Prozor 2
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.0, 2.5+ offset, -3.0));
        m = glm::scale(m, glm::vec3(2.0, 2.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);

        phongShader.setBool("uTransp", false);





        //------------------------------------------------------------------------------------------------------------

        //HUD
        DrawHud(hudShader, hudTex);
        phongShader.use();

        glfwSwapBuffers(window);
        glfwPollEvents();

        FrameEndTime = glfwGetTime();
        params.dt = FrameEndTime - FrameStartTime;
    }

    glfwTerminate();
    return 0;
}


