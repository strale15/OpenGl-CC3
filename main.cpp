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

    glm::vec3 position = glm::vec3(0.0, 4.0, 0.0);
    glm::vec3 objPos = glm::vec3(0.0, 0.0, 0.0);

    double camYaw = 0;
    double camPitch = 0;

    bool wDown = false;
    bool sDown = false;
    bool aDown = false;
    bool dDown = false;

    bool spaceDown = false;
    bool shiftDown = false;

    //House
    float sunAmbient = 0.35;
    float dogRot = 0;

    bool gateOpening = false;
    bool gateClosing = false;
    float gateRot = 0;

    bool light1On = true;
    bool light2On = true;
    bool shuttersOpen = true;

    float sunRot = 0;
    float sunGreen = 0.6;
    bool sunGreenMaxReached = false;

    float dogLightIntensity = 0.6;
    bool dogLightIntensityReached = false;

    float radius = 12;
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
        if (params->isFps) {
            //params->position += 7.2f * params->cameraFront * params->dt;
            params->radius -= 5 * params->dt;
        }     
        else
            params->objPos.z += 0.5f * params->dt;
    }
    if (params->sDown)
    {
        if (params->isFps) {
            params->radius += 5 * params->dt;
            //params->position -= 7.2f * params->cameraFront * params->dt;
        }
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
            params->position.y += 6.0 * params->dt;
        else
            params->objPos.y += 0.5f * params->dt;
    }
    if (params->shiftDown)
    {
        if (params->isFps)
            params->position.y -= 6.0 * params->dt;
        else
            params->objPos.y -= 0.5f * params->dt;
    }

    //House
    params->dogRot += 50 * params->dt;
    if (params->dogRot > 360) {
        params->dogRot -= 360;
    }

    params->sunRot += 20 * params->dt;
    if (params->sunRot > 360) {
        params->sunRot -= 360;
    }

    if (params->gateOpening) {
        params->gateRot += 35 * params->dt;
        params->gateRot = glm::clamp(params->gateRot, 0.f, 80.f);
    }
    if (params->gateClosing) {
        params->gateRot -= 35 * params->dt;
        params->gateRot = glm::clamp(params->gateRot, 0.f, 80.f);
    }

    //Sun pulsing
    if (params->sunGreenMaxReached) {
        params->sunGreen += 0.8 * params->dt;
    }
    else
    {
        params->sunGreen -= 0.8 * params->dt;
    }

    if (params->sunGreen >= 1.0 || params->sunGreen <= 0.5) {
        params->sunGreenMaxReached = !params->sunGreenMaxReached;
    }

    //Dog light pulsing
    if (params->dogLightIntensityReached) {
        params->dogLightIntensity += 3 * params->dt;
    }
    else
    {
        params->dogLightIntensity -= 3 * params->dt;
    }

    if (params->dogLightIntensity >= 2.0 || params->dogLightIntensity <= 0.3) {
        params->dogLightIntensityReached = !params->dogLightIntensityReached;
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

    if (key == GLFW_KEY_UP) {
        if (action == GLFW_PRESS) {
            params->gateOpening = true;
        }
        else if (action == GLFW_RELEASE) {
            params->gateOpening = false;
        }
    }
    if (key == GLFW_KEY_DOWN) {
        if (action == GLFW_PRESS) {
            params->gateClosing = true;
        }
        else if (action == GLFW_RELEASE) {
            params->gateClosing = false;
        }
    }
    if (key == GLFW_KEY_B) {
        if (action == GLFW_PRESS) {
            params->shuttersOpen = true;
        }
    }
    if (key == GLFW_KEY_N) {
        if (action == GLFW_PRESS) {
            params->shuttersOpen = false;
        }
    }
    if (key == GLFW_KEY_1) {
        if (action == GLFW_PRESS) {
            params->light1On = !params->light1On;
        }
    }
    if (key == GLFW_KEY_2) {
        if (action == GLFW_PRESS) {
            params->light2On = !params->light2On;
        }
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

    glfwSetWindowPos(window, 0, 40);
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
    Model bedModel("res/bed.obj");

    Shader phongShader("phong.vert", "phong.frag");
    Shader phongInnerShader("phongInner.vert", "phongInner.frag");
    Shader phongInnerShaderUpper("phongInnerUpper.vert", "phongInnerUpper.frag");
    Shader hudShader("hud.vert", "hud.frag");
    Shader twoD("twoD.vert", "twoD.frag");

    Params params;
    glfwSetWindowUserPointer(window, &params);
    phongShader.use();

    glm::mat4 view;
    glm::mat4 projectionP = glm::perspective(glm::radians(80.0f), (float)wWidth / (float)wHeight, 0.1f, 150.0f);
    phongShader.setMat4("uProjection", projectionP);

    phongShader.setVec3("uDirLight.Position", 0.0, 30, 0.0);
    phongShader.setVec3("uDirLight.Direction", -0.5, -0.5, -0.5);
    phongShader.setVec3("uDirLight.Ka", glm::vec3(params.sunAmbient));
    phongShader.setVec3("uDirLight.Kd", glm::vec3(0.9));
    phongShader.setVec3("uDirLight.Ks", glm::vec3(1.0, 1.0, 1.0));

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

    
    

    glClearColor(0.06, 0.74, 0.99, 1.0);
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
        glm::vec3 camPos = glm::vec3(params.radius * glm::sin(glm::radians(params.camYaw)), params.position.y, params.radius * glm::cos(glm::radians(params.camYaw)));
        view = glm::lookAt(camPos, glm::vec3(0, params.position.y,0), params.cameraUp);
        //view = glm::lookAt(params.position,params.position + params.cameraFront,params.cameraUp);

        phongShader.setMat4("uView", view);
        //phongShader.setVec3("uViewPos", params.position);
        phongShader.setVec3("uViewPos", camPos);

        //SCENE
        //------------------------------------------------------------------------------------------------------------
        //Ground
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
        //m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(35.0, 1.0, 35.0));
        phongShader.setMat4("uModel", m);
        simpleCube2->Render(&phongShader, grassDif, grassSpec);

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

        //Krov
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.55 + offset*2, 0.0));
        m = glm::scale(m, glm::vec3(6.2, 0.1, 6.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 0);


        //UNUTRASNJI OBJEKTI
        phongInnerShader.use();
        phongInnerShader.setMat4("uProjection", projectionP);
        phongInnerShader.setMat4("uView", view);
        phongInnerShader.setVec3("uViewPos", params.position);

        phongInnerShader.setInt("uMaterial.Kd", 0);
        phongInnerShader.setInt("uMaterial.Ks", 1);
        phongInnerShader.setFloat("uMaterial.Shininess", 0.5 * 128);

        phongInnerShader.setVec3("uDirLight.Position", 0.0, 5, 0.0);
        phongInnerShader.setVec3("uDirLight.Direction", 0.4, -1, 0.1);
        phongInnerShader.setVec3("uDirLight.Ka", glm::vec3(params.sunAmbient));
        phongInnerShader.setVec3("uDirLight.Ks", glm::vec3(1.0, 1.0, 1.0));

        phongInnerShader.setVec3("uPointLights[0].Position", glm::vec3(0.0, 0.51-0.1 + offset*2, 0.0));
        phongInnerShaderUpper.setVec3("uPointLights[0].Ka", glm::vec3(0));
        phongInnerShaderUpper.setVec3("uPointLights[0].Kd", glm::vec3(0));
        phongInnerShaderUpper.setVec3("uPointLights[0].Ks", glm::vec3(0));
        phongInnerShaderUpper.setFloat("uPointLights[0].Kc", 1.0f);
        phongInnerShaderUpper.setFloat("uPointLights[0].Kl", 0.7f);
        phongInnerShaderUpper.setFloat("uPointLights[0].Kq", 0.172f);
        if (params.light1On) {
            phongInnerShaderUpper.setVec3("uPointLights[0].Ka", glm::vec3(0.1));
            phongInnerShaderUpper.setVec3("uPointLights[0].Kd", glm::vec3(0.9, 0.79, 0.43) * 6.5f);
            phongInnerShaderUpper.setVec3("uPointLights[0].Ks", glm::vec3(1.0f));
        }

        float innerOffset = 0.15;

        //Pod2(plafon)
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.51 + offset + 0.01, 0.0));
        m = glm::scale(m, glm::vec3(6.0, 0.1, 6.0));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, podDif, podSpec);

        //Plafon
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.5 + offset * 2, 0.0));
        m = glm::scale(m, glm::vec3(5.9, 0.1, 5.9));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, podDif, podSpec);

        //Zadnj zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 2.5 + offset, 3.0- innerOffset));
        m = glm::scale(m, glm::vec3(5.99, 4.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Sa leve strane zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(3.0- innerOffset, 2.5 + offset, 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(6.19, 4.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Sa desne strane zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(-3.0+ innerOffset, 2.5 + offset, 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(6.19, 4.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Zid sa vratima
        m = glm::translate(glm::mat4(1.0), glm::vec3(1.5, 2.5 + offset, -3.0+ innerOffset));
        m = glm::scale(m, glm::vec3(2.99, 4.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Zid ispod prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.49, 1.0 + offset, -3.0+ innerOffset));
        m = glm::scale(m, glm::vec3(3.0, 1.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Zid iznad prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.49, 4.0 + offset, -3.0+ innerOffset));
        m = glm::scale(m, glm::vec3(2.99, 1.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Zid desno od prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-2.5, 2.5 + offset, -3.0+ innerOffset));
        m = glm::scale(m, glm::vec3(0.99, 2.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Krevet
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.5, 0.5 + offset + 0.1, 2.5));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.01));
        phongInnerShader.setMat4("uModel", m);
        bedModel.Draw(phongInnerShader);

        //--------------------------
        //Sprat1Inner
        phongInnerShaderUpper.use();
        phongInnerShaderUpper.setMat4("uProjection", projectionP);
        phongInnerShaderUpper.setMat4("uView", view);
        phongInnerShaderUpper.setVec3("uViewPos", params.position);

        phongInnerShaderUpper.setInt("uMaterial.Kd", 0);
        phongInnerShaderUpper.setInt("uMaterial.Ks", 1);
        phongInnerShaderUpper.setFloat("uMaterial.Shininess", 0.5 * 128);

        phongInnerShaderUpper.setVec3("uDirLight.Position", 0.0, 5, 0.0);
        phongInnerShaderUpper.setVec3("uDirLight.Direction", 0.4, -1, 0.1);
        phongInnerShaderUpper.setVec3("uDirLight.Ka", glm::vec3(params.sunAmbient));
        phongInnerShaderUpper.setVec3("uDirLight.Ks", glm::vec3(1.0, 1.0, 1.0));

        phongInnerShaderUpper.setVec3("uPointLights[0].Position", glm::vec3(0.0, 0.51 - 0.1 + offset, 0.0));
        phongInnerShaderUpper.setVec3("uPointLights[0].Ka", glm::vec3(0));
        phongInnerShaderUpper.setVec3("uPointLights[0].Kd", glm::vec3(0));
        phongInnerShaderUpper.setVec3("uPointLights[0].Ks", glm::vec3(0));
        phongInnerShaderUpper.setFloat("uPointLights[0].Kc", 1.0f);
        phongInnerShaderUpper.setFloat("uPointLights[0].Kl", 0.7f);
        phongInnerShaderUpper.setFloat("uPointLights[0].Kq", 0.172f);
        if (params.light2On) {
            phongInnerShaderUpper.setVec3("uPointLights[0].Ka", glm::vec3(0.1));
            phongInnerShaderUpper.setVec3("uPointLights[0].Kd", glm::vec3(0.9, 0.79, 0.43) * 6.5f);
            phongInnerShaderUpper.setVec3("uPointLights[0].Ks", glm::vec3(1.0f));
        }

        //Pod2 (plafon prve sobe)
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.51 + offset, 0.0));
        m = glm::scale(m, glm::vec3(6.0, 0.1, 6.0));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, podDif, podSpec);

        //Pod
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.51, 0.0));
        m = glm::scale(m, glm::vec3(6.0, 0.1, 6.0));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, podDif, podSpec);

        //Zadnj zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 2.5 , 3.0 - innerOffset));
        m = glm::scale(m, glm::vec3(5.99, 4.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Sa leve strane zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(3.0 - innerOffset, 2.5 , 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(6.19, 4.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Sa desne strane zid
        m = glm::translate(glm::mat4(1.0), glm::vec3(-3.0 + innerOffset, 2.5 , 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(6.19, 4.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Zid sa vratima
        m = glm::translate(glm::mat4(1.0), glm::vec3(1.5, 2.5 , -3.0 + innerOffset));
        m = glm::scale(m, glm::vec3(2.99, 4.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Zid ispod prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.49, 1.0 , -3.0 + innerOffset));
        m = glm::scale(m, glm::vec3(3.0, 1.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Zid iznad prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.49, 4.0 , -3.0 + innerOffset));
        m = glm::scale(m, glm::vec3(2.99, 1.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Zid desno od prozora
        m = glm::translate(glm::mat4(1.0), glm::vec3(-2.5, 2.5 , -3.0 + innerOffset));
        m = glm::scale(m, glm::vec3(0.99, 2.0, 0.1));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, tapeteDif);

        //Sto za bice
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.0, 0.5 + 0.4, -2.0));
        m = glm::scale(m, glm::vec3(1.6, 0.8, 1.0));
        phongInnerShader.setMat4("uModel", m);
        simpleCube->Render(&phongInnerShader, 0.36, 0.23, 0.03);

        //Bice (Dingus)
        glDisable(GL_CULL_FACE);
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.0, 0.5 + 0.8, -2.1));
        m = glm::rotate(m, glm::radians(130.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.03));
        phongInnerShader.setMat4("uModel", m);
        maxwell.Draw(phongInnerShader);
        glEnable(GL_CULL_FACE);


        //------------------------------------------------------------------------------------------------
        //OSTALE STVARI VAN
        phongShader.use();
        //Vrata
        m = glm::translate(glm::mat4(1.0), glm::vec3(1.5, 1.6, -3.0-innerOffset/2));
        m = glm::scale(m, glm::vec3(1.2, 2.2, 0.1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.46, 0.25, 0.11);

        phongShader.setVec3("uSpotlights[0].Position", glm::vec3(1.5, 2.2+0.2, -3.0 - innerOffset / 2));
        phongShader.setVec3("uSpotlights[0].Direction", 0.0, -0.5, -0.5);
        phongShader.setVec3("uSpotlights[0].Ka", 0.0, 0.0, 0.0);
        phongShader.setVec3("uSpotlights[0].Kd", glm::vec3(2.0f, 2.0f, 2.0f));
        phongShader.setVec3("uSpotlights[0].Ks", glm::vec3(1.0));
        phongShader.setFloat("uSpotlights[0].InnerCutOff", glm::cos(glm::radians(20.0f)));
        phongShader.setFloat("uSpotlights[0].OuterCutOff", glm::cos(glm::radians(27.0f)));
        phongShader.setFloat("uSpotlights[0].Kc", 1.0);
        phongShader.setFloat("uSpotlights[0].Kl", 0.052f);
        phongShader.setFloat("uSpotlights[0].Kq", 0.032f);

        //Dimnjak
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.5, 0.55 + offset * 2 + 1, 1.5));
        m = glm::scale(m, glm::vec3(1.2, 2.0, 1.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.88, 0.48, 0.05);

        //Sunce
        m = glm::translate(glm::mat4(1.0), glm::vec3(10.0, 20.0, 10.0));
        m = glm::rotate(m, glm::radians(params.sunRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(3.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, params.sunGreen, 0);

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
        m = glm::rotate(m, glm::radians(params.gateRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -glm::vec3(1.0, 1.0, -6.0));

        m = glm::translate(m, glm::vec3(0.0, 1.0, -6.0));
        m = glm::scale(m, glm::vec3(2, 0.9, 0.11));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.85, 0.65, 0.36);

        //Drvo
        m = glm::translate(glm::mat4(1.0), glm::vec3(-3.0, 0.5, -5.0));
        m = glm::scale(m, glm::vec3(0.5));
        phongShader.setMat4("uModel", m);
        treeObj.Draw(phongShader); 

        //Pas
        glDisable(GL_CULL_FACE);
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

        phongShader.setVec3("uPointLights[0].Position", glm::vec3(m[3]));
        phongShader.setVec3("uPointLights[0].Ka", glm::vec3(0.1));
        phongShader.setVec3("uPointLights[0].Kd", glm::vec3(2.1f)*params.dogLightIntensity);
        phongShader.setVec3("uPointLights[0].Ks", glm::vec3(1.0f));
        phongShader.setFloat("uPointLights[0].Kc", 1.5f);
        phongShader.setFloat("uPointLights[0].Kl", 1.0f);
        phongShader.setFloat("uPointLights[0].Kq", 0.272f);
        
        //TRANSPARENTNE STVARI
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

        if(params.shuttersOpen)
            phongShader.setFloat("uAlpha", 0.25);
        else
            phongShader.setFloat("uAlpha", 0.98);
        //Prozor 1
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.0, 2.5, -3.0));
        m = glm::scale(m, glm::vec3(2.0, 2.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.04, 0.71, 0.99);

        //Prozor 2
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.0, 2.5+ offset, -3.0));
        m = glm::scale(m, glm::vec3(2.0, 2.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.32, 0.02, 0.05);

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


