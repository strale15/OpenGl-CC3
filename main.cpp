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

#include <cmath>
#include <Windows.h>

#include <stdlib.h>     /* srand, rand */
#include <time.h> 

#define TARGET_NUMBER 10

const unsigned int wWidth = 1920;
const unsigned int wHeight = 1080;

bool firstMouse = true;
double lastX;
double lastY;

struct Target {
    glm::vec3 targetPosition;
    int targetId;
    bool alive;
};

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

    //Tank
    glm::vec3 turretForward = glm::vec3(0.0, 0.0, 1.0);
    glm::vec3 turretUp = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 turretRight = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 muzzlePos = glm::vec3(0.0, 0.0, 0.0);

    bool rotLeft = false;
    bool rotRight = false;
    bool rotUp = false;
    bool rotDown = false;

    float rotY = 0;
    float rotX = 0;

    float chargeTime = 0;
    bool isCharged = true;
    int ammo = 10;
    bool firedThisFrame = false;

    bool isScope = false;

    bool voltageUp = false;
    bool voltageDown = false;
    float voltage = 10;

    bool nightVision = false;
    float zoom = 90;

    float afterFire = 0;

    bool isLightOn = false;
};

Target targets[TARGET_NUMBER];

float targetScale = 3;

//float randomInnacuracy = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / params.voltage) - params.voltage/2;
static void GenerateTargetPositions() {
    for (int i = 0; i < TARGET_NUMBER; i++) {
    gas:
        float randomX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 100.0) - 50.0;
        float randomZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 100.0) - 50.0;
        float Y = targetScale/2+0.5;

        if (glm::distance(glm::vec3(randomX, Y, randomZ), glm::vec3(0)) < 20) {
            goto gas;
        }

        targets[i].targetPosition = glm::vec3(randomX, Y, randomZ);
        targets[i].targetId = i;
        targets[i].alive = true;

        cout << randomX << " " << randomZ << endl;
    }
}

static void RenderAliveTargets(Shader& shader, GameObject* targetObj) {
    for (int i = 0; i < TARGET_NUMBER; i++) {
        if (targets[i].alive) {
            glm::mat4 m;
            m = glm::translate(glm::mat4(1.0), targets[i].targetPosition);
            //m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(1.0, targetScale, 1.0));
            shader.setMat4("uModel", m);
            targetObj->Render(&shader, 1, 0, 0);
        }
    }
}

static void CheckIfTargetHit(Params* params) {
    for (int i = 0; i < TARGET_NUMBER; i++) {
        if (targets[i].alive) {
            glm::vec3 targetPos = targets[i].targetPosition;
            glm::vec3 direction = targetPos - params->muzzlePos;
            float dotProduct = glm::dot(glm::normalize(direction), params->turretForward);
            dotProduct = glm::clamp(dotProduct, -1.f, 1.f);
            float angleRadians = std::acos(dotProduct);
            float angleDegrees = glm::degrees(angleRadians);
            //cout << targets[i].targetId << " " << angleDegrees << endl;
            if (angleDegrees <= 4) {
                targets[i].alive = false;
            }
        }
    }
}

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

    //Tank
    if (params->rotLeft) {
        params->rotY += 8 * params->voltage * params->dt;
    }
    if (params->rotRight) {
        params->rotY -= 8 * params->voltage * params->dt;
    }
    if (params->rotUp) {
        params->rotX += 10 * params->dt;
        params->rotX = glm::clamp(params->rotX, -10.f, 10.f);
    }
    if (params->rotDown) {
        params->rotX -= 10 * params->dt;
        params->rotX = glm::clamp(params->rotX, -10.f, 10.f);
    }

    if (params->voltageUp) {
        params->voltage += 2 * params->dt;
        params->voltage = glm::clamp(params->voltage, 0.f, 10.f);
    }
    if (params->voltageDown) {
        params->voltage -= 2 * params->dt;
        params->voltage = glm::clamp(params->voltage, 0.f, 10.f);
    }

    if (!params->isCharged) {
        if (params->chargeTime >= 7.5) {
            params->isCharged = true;
            params->chargeTime = 0;
        }
        else
        {
            params->chargeTime += params->dt;
            params->afterFire += params->dt;
        }
        
    }

    if (params->afterFire > 0.1 && params->isCharged) {
        if (params->afterFire >= 10) {
            params->afterFire = 0;
        }
        else
        {
            params->afterFire += params->dt;
        }
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

    //Tank
    if (key == GLFW_KEY_LEFT) {
        if (action == GLFW_PRESS) {
            params->rotLeft = true;
        }
        else if (action == GLFW_RELEASE) {
            params->rotLeft = false;
        }
    }
    if (key == GLFW_KEY_RIGHT) {
        if (action == GLFW_PRESS) {
            params->rotRight = true;
        }
        else if (action == GLFW_RELEASE) {
            params->rotRight = false;
        }
    }
    if (key == GLFW_KEY_UP) {
        if (action == GLFW_PRESS) {
            params->rotUp = true;
        }
        else if (action == GLFW_RELEASE) {
            params->rotUp = false;
        }
    }
    if (key == GLFW_KEY_DOWN) {
        if (action == GLFW_PRESS) {
            params->rotDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->rotDown = false;
        }
    }

    if (key == GLFW_KEY_KP_ADD) {
        if (action == GLFW_PRESS) {
            params->voltageUp = true;
        }
        else if (action == GLFW_RELEASE) {
            params->voltageUp = false;
        }
    }

    if (key == GLFW_KEY_KP_SUBTRACT) {
        if (action == GLFW_PRESS) {
            params->voltageDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->voltageDown = false;
        }
    }

    if (key == GLFW_KEY_V) {
        if (action == GLFW_PRESS) {
            params->isScope = true;
        }
    }
    if (key == GLFW_KEY_C) {
        if (action == GLFW_PRESS) {
            params->isScope = false;
        }
    }

    if (key == GLFW_KEY_F) {
        if (action == GLFW_PRESS) {
            params->isLightOn = !params->isLightOn;
        }
    }

    if (key == GLFW_KEY_G) {
        if (action == GLFW_PRESS) {
            params->nightVision = !params->nightVision;
        }
    }

    if (key == GLFW_KEY_L) {
        if (action == GLFW_PRESS) {
            //Nesto nesto?
            if (params->isCharged && params->ammo > 0) {
                params->ammo -= 1;
                params->isCharged = false;
                params->firedThisFrame = true;
                params->afterFire = 0;
            }
        }
    }

    if (key == GLFW_KEY_1) {
        if (action == GLFW_PRESS) {
            params->zoom = 90;
            cout << "lol" << endl;
        }
    }
    if (key == GLFW_KEY_2) {
        if (action == GLFW_PRESS) {
            params->zoom = 60;
        }
    }
    if (key == GLFW_KEY_3) {
        if (action == GLFW_PRESS) {
            params->zoom = 30;
        }
    }
}

std::vector<float> generateHalfCircleVertices(float radius, int numSegments) {
    std::vector<float> vertices;

    // Ensure an even number of segments for a smooth half circle
    if (numSegments % 2 != 0) {
        numSegments++;
    }

    for (int i = 0; i < numSegments-1; ++i) {
        float theta1 = M_PI * i / (numSegments - 1);
        float theta2 = M_PI * (i + 1) / (numSegments - 1);

        // Calculate the vertices for the current triangle
        float x1 = radius * std::cos(theta1);
        float y1 = radius * std::sin(theta1);
        float x2 = radius * std::cos(theta2);
        float y2 = radius * std::sin(theta2);

        // Add the vertices to the vector
        vertices.push_back(0.0f);  // Center vertex
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.5f);  // Center vertex UV
        vertices.push_back(0.5f);

        vertices.push_back(x1);
        vertices.push_back(y1);
        vertices.push_back(0.0f);
        vertices.push_back(0.5f * (x1 / radius) + 0.5f);
        vertices.push_back(0.5f * (y1 / radius) + 0.5f);

        vertices.push_back(x2);
        vertices.push_back(y2);
        vertices.push_back(0.0f);
        vertices.push_back(0.5f * (x2 / radius) + 0.5f);
        vertices.push_back(0.5f * (y2 / radius) + 0.5f);
    }

    return vertices;
}

int main()
{
    srand(time(NULL));

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

    std::vector<float> vertices = {
        // Positions      // UVs
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,  // Vertex 1
        0.5f, -0.5f, 0.0f, 1.0f, 1.0f,  // Vertex 2
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f,  // Vertex 3

        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,  // Vertex 1 (Repeated)
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f,  // Vertex 3 (Repeated)
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f   // Vertex 4
    };
    GameObject* rectangle = new GameObject(vertices, true);

    std::vector<float> circleVert = generateHalfCircleVertices(1, 64);
    GameObject* halfCircle = new GameObject(circleVert, true);

    Model lija("res/low-poly-fox.obj");

    Shader phongShader("phong.vert", "phong.frag");
    Shader hudShader("hud.vert", "hud.frag");
    Shader twoD("twoD.vert", "twoD.frag");

    phongShader.use();

    glm::mat4 view;
    glm::mat4 projectionP;

    phongShader.setVec3("uDirLight.Position", 0.0, 30, 0.0);
    phongShader.setVec3("uDirLight.Direction", 0.2, -1, 0.2);
    phongShader.setVec3("uDirLight.Ka", glm::vec3(0.2));
    phongShader.setVec3("uDirLight.Kd", glm::vec3(0.6));
    phongShader.setVec3("uDirLight.Ks", glm::vec3(1.0, 1.0, 1.0));

    unsigned hudTex = Model::textureFromFile("res/hudTex.png");
    unsigned kockaDif = Model::textureFromFile("res/container_diffuse.png");
    unsigned kockaSpec = Model::textureFromFile("res/container_specular.png");

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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    GenerateTargetPositions();

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
        projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);

        phongShader.setMat4("uProjection", projectionP);
        phongShader.setMat4("uView", view);
        phongShader.setVec3("uViewPos", params.position);

        //SCENE
        //------------------------------------------------------------------------------------------------------------
        RenderAliveTargets(phongShader, simpleCube);
        if (params.firedThisFrame) {
            CheckIfTargetHit(&params);
        }

        if (params.nightVision && params.isScope) {
            phongShader.setVec3("uDirLight.Position", 0.0, 30, 0.0);
            phongShader.setVec3("uDirLight.Direction", 0.2, -1, 0.2);
            phongShader.setVec3("uDirLight.Ka", glm::vec3(0,0.1,0));
            phongShader.setVec3("uDirLight.Kd", glm::vec3(0,0.3,0));
            phongShader.setVec3("uDirLight.Ks", glm::vec3(0,1.0, 0.0));
        }
        else
        {
            phongShader.setVec3("uDirLight.Position", 0.0, 30, 0.0);
            phongShader.setVec3("uDirLight.Direction", 0.2, -1, 0.2);
            phongShader.setVec3("uDirLight.Ka", glm::vec3(0.2));
            phongShader.setVec3("uDirLight.Kd", glm::vec3(0.6));
            phongShader.setVec3("uDirLight.Ks", glm::vec3(1.0, 1.0, 1.0));
        }
        
        //Ground
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
        //m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(100.0, 1.0, 100.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 0.5, 1);

        //Tank Base
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.5+0.6, 0.0));
        m = glm::scale(m, glm::vec3(5.0, 1.2, 7.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 1, 0);

        //Kupola1
        m = glm::rotate(glm::mat4(1.0), glm::radians(params.rotY), glm::vec3(0.0, 1.0, 0.0));

        m = glm::translate(m, glm::vec3(0.0, 0.5 + 1.2 + 0.7, -3.5/2));
        m = glm::scale(m, glm::vec3(2.5, 1.4, 0.05));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 0, 1);

        //Kupola2
        m = glm::rotate(glm::mat4(1.0), glm::radians(params.rotY), glm::vec3(0.0, 1.0, 0.0));

        m = glm::translate(m, glm::vec3(0.0, 0.5 + 1.2 + 0.7, 3.5 / 2));
        m = glm::scale(m, glm::vec3(2.5, 1.4, 0.05));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 0, 1);

        glm::vec3 spotPos = glm::vec3(m[3]);
        glm::vec3 spotDir = glm::normalize(glm::vec3(m[2]));
        glm::vec3 spotRight = glm::cross(spotDir, glm::vec3(0, 1, 0));

        glm::vec3 lightInt = glm::vec3(0);
        glm::vec3 lightIntS = glm::vec3(0);
        if (params.isLightOn) {
            lightInt = glm::vec3(2.f);
            lightIntS = glm::vec3(1.f);
        }

        phongShader.setVec3("uSpotlights[0].Position", spotPos + spotRight*1.1f);
        phongShader.setVec3("uSpotlights[0].Direction", spotDir);
        phongShader.setVec3("uSpotlights[0].Ka", 0.0, 0.0, 0.0);
        phongShader.setVec3("uSpotlights[0].Kd", lightInt);
        phongShader.setVec3("uSpotlights[0].Ks", lightIntS);
        phongShader.setFloat("uSpotlights[0].InnerCutOff", glm::cos(glm::radians(20.0f)));
        phongShader.setFloat("uSpotlights[0].OuterCutOff", glm::cos(glm::radians(25.0f)));
        phongShader.setFloat("uSpotlights[0].Kc", 1.0);
        phongShader.setFloat("uSpotlights[0].Kl", 0.92f);
        phongShader.setFloat("uSpotlights[0].Kq", 0.032f);

        //Kupola3
        m = glm::rotate(glm::mat4(1.0), glm::radians(params.rotY), glm::vec3(0.0, 1.0, 0.0));

        m = glm::translate(m, glm::vec3(2.5/2, 0.5 + 1.2 + 0.7, 0.0));
        m = glm::scale(m, glm::vec3(0.05, 1.4, 3.5));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 0, 1);

        //Kupola4
        m = glm::rotate(glm::mat4(1.0), glm::radians(params.rotY), glm::vec3(0.0, 1.0, 0.0));

        m = glm::translate(m, glm::vec3(-2.5 / 2, 0.5 + 1.2 + 0.7, 0.0));
        m = glm::scale(m, glm::vec3(0.05, 1.4, 3.5));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 0, 1);

        //Kupola5
        m = glm::rotate(glm::mat4(1.0), glm::radians(params.rotY), glm::vec3(0.0, 1.0, 0.0));

        m = glm::translate(m, glm::vec3(0.0, 0.5 + 1.2 + 0.7+0.7, 0.0));
        m = glm::scale(m, glm::vec3(2.5, 0.05, 3.5));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 0, 0);

        //Kupola6 (top)
        glm::vec3 turretCenter = glm::vec3(0.0, 0.5 + 1.2 + 0.7, 0.0);

        m = glm::translate(glm::mat4(1.0), turretCenter);
        m = glm::rotate(m, glm::radians(params.rotY), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -turretCenter);

        m = glm::translate(m, glm::vec3(0.0, 0.5 + 1.2 + 0.7, 7.0/2+3.5/2));
        m = glm::scale(m, glm::vec3(0.4, 0.4, 7.5));

        glm::vec3 forward = glm::vec3(m[2]);
        forward = glm::normalize(forward);

        glm::vec3 right = glm::cross(forward, glm::vec3(0, 1, 0));
        right = glm::normalize(right);

        glm::vec3 objPos = glm::vec3(m[3]);
        objPos -= forward * 7.5f/2.f;

        

        m = glm::translate(glm::mat4(1.0), objPos);
        m = glm::rotate(m, glm::radians(params.rotX), right);
        m = glm::translate(m, -objPos);

        m = glm::translate(m, turretCenter);
        m = glm::rotate(m, glm::radians(params.rotY), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -turretCenter);
        
        m = glm::translate(m, glm::vec3(0.0, 0.5 + 1.2 + 0.7, 7.0 / 2 + 3.5 / 2));
        m = glm::scale(m, glm::vec3(0.4, 0.4, 7.5));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 0, 0);

        forward = glm::vec3(m[2]);
        forward = glm::normalize(forward);

        objPos = glm::vec3(m[3]);
        objPos += forward * 7.5f / 2.f;

        params.turretForward = forward;
        params.muzzlePos = objPos;
        params.turretUp = glm::vec3(0, 1, 0);
        params.turretRight = glm::cross(forward, params.turretUp);



        m = glm::translate(glm::mat4(1.0), objPos);
        m = glm::scale(m, glm::vec3(0.6));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 1, 1);

        //Meta
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 1.5, 25.0));
        m = glm::scale(m, glm::vec3(1,2,1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 1, 1);

        //Formula za metu
        glm::vec3 targetPos = glm::vec3(0.0, 1.5, 25.0);
        glm::vec3 direction = targetPos - params.muzzlePos;
        float dotProduct = glm::dot(glm::normalize(direction), params.turretForward);
        dotProduct = glm::clamp(dotProduct, -1.f, 1.f);
        float angleRadians = std::acos(dotProduct);
        float angleDegrees = glm::degrees(angleRadians);

        if (params.firedThisFrame) {
            cout << "puc " << angleDegrees << " " << endl;
        }
        if (params.firedThisFrame && angleDegrees <= 5) {
            cout << "hit" << endl;
        }

        //Muzzle flash
        glm::vec3 muzzleLightInt = glm::vec3(0);
        glm::vec3 muzzleLightIntS = glm::vec3(0);

        if (params.afterFire != 0) {
            float intCoef = 0.1 - params.afterFire;
            if (intCoef < 0) {
                intCoef = 0;
            }
            muzzleLightInt = glm::vec3(1, 1, 0) * intCoef * 3000.f;
            muzzleLightIntS = glm::vec3(1, 1, 0) * intCoef * 10.f;
        }

        phongShader.setVec3("uPointLights[0].Position", params.muzzlePos);
        phongShader.setVec3("uPointLights[0].Ka", muzzleLightInt/10.f);
        phongShader.setVec3("uPointLights[0].Kd", muzzleLightInt);
        phongShader.setVec3("uPointLights[0].Ks", muzzleLightIntS);
        phongShader.setFloat("uPointLights[0].Kc", 1.5f);
        phongShader.setFloat("uPointLights[0].Kl", 4.0f);
        phongShader.setFloat("uPointLights[0].Kq", 4.272f);
        //------------------------------------------------------------------------------------------------------------

        if (params.firedThisFrame) {
            params.firedThisFrame = false;
        }

        //2D Stvari
        twoD.use();
        twoD.setMat4("uProjection", projectionP);
        twoD.setMat4("uView", view);

        spotPos -= spotDir * 0.05f;

        //base inst
        m = glm::rotate(glm::mat4(1.0), glm::radians(params.rotY), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, glm::vec3(0.0, 0.5 + 1.2 + 0.7, 3.5 / 2 - 0.1));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(1.0, 1.0, 1.0));
        twoD.setMat4("uModel", m);
        //rectangle->Render(&twoD, 1, 0.5, 1);

        for (int i = 0; i < params.ammo; i++) {
            m = glm::rotate(glm::mat4(1.0), glm::radians(params.rotY), glm::vec3(0.0, 1.0, 0.0));
            m = glm::translate(m, glm::vec3(0.0, 0.5 + 1.2 + 0.7, 3.5 / 2 - 0.11));
            m = glm::translate(m, glm::vec3(1.1-0.06*i, -0.4, 0.0));
            m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(0.03, 0.2, 1.0));
            twoD.setMat4("uModel", m);
            rectangle->Render(&twoD, 1, 1, 0);
        }

        m = glm::rotate(glm::mat4(1.0), glm::radians(params.rotY), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, glm::vec3(0.0, 0.5 + 1.2 + 0.7, 3.5 / 2 - 0.1));
        m = glm::translate(m, glm::vec3(-0.7, -0.3,0));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.2));
        twoD.setMat4("uModel", m);
        if (params.isCharged) {
            rectangle->Render(&twoD, 0, 1, 0);
        }
        else
        {
            rectangle->Render(&twoD, 1, 1, 1);
        }

        m = glm::rotate(glm::mat4(1.0), glm::radians(params.rotY), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, glm::vec3(0.0, 0.5 + 1.2 + 0.7, 3.5 / 2 - 0.1));
        m = glm::translate(m, glm::vec3(-0.7, -0.1, 0));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.2));
        twoD.setMat4("uModel", m);
        halfCircle->Render(&twoD, 1, 0.5, 1);

        //Pointer
        float randomInnacuracy = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / params.voltage) - params.voltage/2;

        m = glm::rotate(glm::mat4(1.0), glm::radians(params.rotY), glm::vec3(0.0, 1.0, 0.0));

        m = glm::translate(m, glm::vec3(0.0, 0.5 + 1.2 + 0.7, 3.5 / 2 - 0.11));
        m = glm::translate(m, glm::vec3(-0.7, -0.1, 0));

        float rotation = params.voltage * 18 - 90 + randomInnacuracy;
        rotation = glm::clamp(rotation, -90.f, 90.f);
        m = glm::rotate(m, glm::radians(rotation), glm::vec3(0.0, 0.0, 1.0));

        m = glm::translate(m, -glm::vec3(0.0, 0.5 + 1.2 + 0.7, 3.5 / 2 - 0.11));
        m = glm::translate(m, -glm::vec3(-0.7, -0.1, 0));

        m = glm::translate(m, glm::vec3(0.0, 0.5 + 1.2 + 0.7, 3.5 / 2 - 0.11));
        m = glm::translate(m, glm::vec3(-0.7, -0.1+0.075, 0));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.015,0.15,1));
        twoD.setMat4("uModel", m);
        rectangle->Render(&twoD, 1, 0, 0);


        //Transparentne stvari
        if (params.afterFire != 0) {
            phongShader.use();
            phongShader.setBool("uTransp", true);
            phongShader.setFloat("uAlpha", 1-params.afterFire/10.f);

            m = glm::translate(glm::mat4(1.0), params.muzzlePos + params.turretForward * 1.6f - params.turretRight + params.turretUp * 0.5f);
            m = glm::scale(m, glm::vec3(0.5));
            phongShader.setMat4("uModel", m);
            simpleCube->Render(&phongShader, 1, 1, 1);

            m = glm::translate(glm::mat4(1.0), params.muzzlePos + params.turretForward*2.f + params.turretRight - params.turretUp * 0.5f);
            m = glm::scale(m, glm::vec3(0.4));
            phongShader.setMat4("uModel", m);
            simpleCube->Render(&phongShader, 1, 1, 1);

            m = glm::translate(glm::mat4(1.0), params.muzzlePos + params.turretForward + params.turretRight);
            m = glm::scale(m, glm::vec3(0.6));
            phongShader.setMat4("uModel", m);
            simpleCube->Render(&phongShader, 1, 1, 1);

            phongShader.setBool("uTransp", false);
        }

        //HUD
        DrawHud(hudShader, hudTex);

        glfwSwapBuffers(window);
        glfwPollEvents();

        FrameEndTime = glfwGetTime();
        params.dt = FrameEndTime - FrameStartTime;
    }

    glfwTerminate();
    return 0;
}


