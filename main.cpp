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

const unsigned int wWidth = 1920;
const unsigned int wHeight = 1080;

bool firstMouse = true;
double lastX;
double lastY;

struct Params {
    float dt;
    bool test1 = false;

    bool isCurosIn;
    double xPosC = 0.0;
    double yPosC = 0.0;

    double frontX;
    double frontY;
    double frontZ;
    glm::vec3 cameraFront = glm::vec3(0.0, -0.5, 0.866025);
    glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

    glm::vec3 position = glm::vec3(0.0, 0.0, 3.0);
    glm::vec3 objPos = glm::vec3(0.0, 1.0, 0.0);

    double camYaw = 90;
    double camPitch = -30;

    float camX;
    float camY;
    float camZ;

    bool wDown = false;
    bool sDown = false;
    bool aDown = false;
    bool dDown = false;

    bool spaceDown = false;
    bool shiftDown = false;

    bool nightVision = false;
    bool headlights = false;

    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool rotLeft = false;
    bool rotRight = false;

    bool engineBroken = false;
    bool bateryBroken = false;

    float velocity = 0;
    glm::vec3 carOffset = glm::vec3(0);
    glm::vec3 carForward = glm::vec3(0);
    float carXOffset = 0;
    float carRot = 0;
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

static void DrawBuildings(Shader& shader, GameObject* building, unsigned buildingTex, unsigned buildingSpec) {
    shader.use();
    int numberOfBuildings = 2000/20;
    glm::mat4 m = glm::mat4(1.f);

    for (int i = 0; i < numberOfBuildings; i++) {
        float offset = 20.f * i;
        m = glm::translate(glm::mat4(1.0), glm::vec3(40.0, 23.0, -1000.0 + offset));
        m = glm::rotate(m, glm::radians(0.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(10.0, 50.0, 10.0));
        shader.setMat4("uModel", m);
        building->Render(&shader, buildingTex, buildingSpec);

        m = glm::translate(glm::mat4(1.0), glm::vec3(-40.0, 23.0, -1000.0 + offset));
        m = glm::rotate(m, glm::radians(0.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(10.0, 50.0, 10.0));
        shader.setMat4("uModel", m);
        building->Render(&shader, buildingTex, buildingSpec);
    }
}

static void HandleInput(Params* params) {
    if (params->wDown)
    {
        if (params->test1)
            params->position += 7.2f * params->cameraFront * params->dt;
        else
            params->objPos.z += 0.5f * params->dt;
    }
    if (params->sDown)
    {
        if (params->test1)
            params->position -= 7.2f * params->cameraFront * params->dt;
        else
            params->objPos.z -= 0.5f * params->dt;
    }
    if (params->aDown)
    {

        glm::vec3 strafe = glm::cross(params->cameraFront, params->cameraUp);
        if (params->test1)
            params->position -= 7.2f * strafe * params->dt;
        else
            params->objPos.x += 0.5f * params->dt;
    }
    if (params->dDown)
    {
        glm::vec3 strafe = glm::cross(params->cameraFront, params->cameraUp);
        if (params->test1)
            params->position += 7.2f * strafe * params->dt;
        else
            params->objPos.x -= 0.5f * params->dt;
    }
    if (params->spaceDown)
    {
        if (params->test1)
            params->position.y += 4.2 * params->dt;
        else
            params->objPos.y += 0.5f * params->dt;
    }
    if (params->shiftDown)
    {
        if (params->test1)
            params->position.y -= 4.1 * params->dt;
        else
            params->objPos.y -= 0.5f * params->dt;
    }


    //Car
    params->velocity = glm::clamp(params->velocity, -20.f, 60.f);
    //cout << params->velocity << endl;

    if (glm::abs(params->velocity) < 0.005) {
        params->velocity = 0;
    }

    if (params->velocity > 0 && !params->forward) {
        params->velocity -= 8 * params->dt;
    }
    else if (params->velocity < 0 && !params->backward) {
        params->velocity += 8 * params->dt;
    }

    if (params->forward) {
        params->velocity += 10 * params->dt;
    }
    if (params->backward) {
        params->velocity -= 10 * params->dt;
    }
    if (params->left) {
        glm::vec3 left = glm::cross(glm::vec3(0, 1, 0), params->carForward);
        left = glm::normalize(left);

        params->carOffset += 0.5f * left * glm::abs(params->velocity) * params->dt;
        params->carOffset += 0.2f * params->carForward * params->velocity * params->dt;
    }
    else if (params->right) {
        glm::vec3 right = -glm::cross(glm::vec3(0, 1, 0), params->carForward);
        right = glm::normalize(right);

        params->carOffset += 0.5f * right * glm::abs(params->velocity) * params->dt;
        params->carOffset += 0.2f * params->carForward * params->velocity * params->dt;
    }
    else {
        params->carOffset += params->carForward * params->velocity * params->dt;
    }

    params->carOffset.z = glm::clamp(params->carOffset.z, -1000.f, 1000.f);
    params->carOffset.x = glm::clamp(params->carOffset.x, -25.f, 25.f);

    if (params->rotLeft) {
        params->carRot += 120 * params->dt;
    }
    if (params->rotRight) {
        params->carRot -= 120 * params->dt;
    }

    params->carRot = glm::clamp(params->carRot, -720.f, 720.f);
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
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        std::cout << "glm::vec3(" << params->objPos.x << "," << params->objPos.y << "," << params->objPos.z << ")" << std::endl;
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        params->test1 = !params->test1;
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

    //CAR
    if (key == GLFW_KEY_UP) {
        if (action == GLFW_PRESS) {
            params->forward = true;
        }
        else if (action == GLFW_RELEASE) {
            params->forward = false;
        }
    }

    if (key == GLFW_KEY_DOWN) {
        if (action == GLFW_PRESS) {
            params->backward = true;
        }
        else if (action == GLFW_RELEASE) {
            params->backward = false;
        }
    }

    if (key == GLFW_KEY_LEFT) {
        if (action == GLFW_PRESS) {
            params->left = true;
        }
        else if (action == GLFW_RELEASE) {
            params->left = false;
        }
    }
    if (key == GLFW_KEY_RIGHT) {
        if (action == GLFW_PRESS) {
            params->right = true;
        }
        else if (action == GLFW_RELEASE) {
            params->right = false;
        }
    }
    if (key == GLFW_KEY_Q) {
        if (action == GLFW_PRESS) {
            params->rotLeft = true;
        }
        else if (action == GLFW_RELEASE) {
            params->rotLeft = false;
        }
    }
    if (key == GLFW_KEY_E) {
        if (action == GLFW_PRESS) {
            params->rotRight = true;
        }
        else if (action == GLFW_RELEASE) {
            params->rotRight = false;
        }
    }
    if (key == GLFW_KEY_R) {
        if (action == GLFW_PRESS) {
            params->nightVision = !params->nightVision;
        }
    }
    if (key == GLFW_KEY_F) {
        if (action == GLFW_PRESS) {
            params->headlights = !params->headlights;
        }
    }
    if (key == GLFW_KEY_C) {
        if (action == GLFW_PRESS) {
            params->engineBroken = !params->engineBroken;
        }
    }
    if (key == GLFW_KEY_B) {
        if (action == GLFW_PRESS) {
            params->bateryBroken = !params->bateryBroken;
        }
    }

    bool IsDown = action == GLFW_PRESS || action == GLFW_REPEAT;
    switch (key) {
    case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    }
}


int main()
{
    if(!glfwInit())
    {
        std::cout << "GLFW fail!\n" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 16);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_CLAMP);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    GLFWwindow* window = glfwCreateWindow(wWidth, wHeight, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Window fail!\n" << std::endl;
        glfwTerminate();
        return -2;
    }
    glfwSetWindowPos(window, 300, 100);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursosPosCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() !=GLEW_OK)
    {
        std::cout << "GLEW fail! :(\n" << std::endl;
        return -3;
    }

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

    std::vector<float> cubeVertices2 = {
        // X     Y     Z     NX    NY    NZ    U     V    FRONT SIDE
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 100.0f, // L D
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 100.0f, 100.0f, // R D
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // L U
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 100.0f, 100.0f, // R D
        0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // R U
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // L U
        // LEFT SIDE
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 100.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 100.0f, // L U
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 100.0f, 0.0f, // R D
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 100.0f, 100.0f, // R U
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 100.0f, // L U
        // RIGHT SIDE
        0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, // R D
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 100.0f, // L U
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, // R D
        0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 100.0f, 100.0f, // R U
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 100.0f, // L U
        // BOTTOM SIDE
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 100.0f, 0.0f, // R D
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 100.0f, // L U
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 100.0f, 0.0f, // R D
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 100.0f, 100.0f, // R U
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 100.0f, // L U
        // TOP SIDE
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 2.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f * (2000) / 70, // L U
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 2.0f, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 2.0f, 2.0f * (2000)/ 70, // R U
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f * (2000) / 70, // L U
        // BACK SIDE
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 100.0f, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 100.0f, // L U
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 100.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 100.0f, 100.0f, // R U
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 100.0f, // L U
    };
    GameObject* simpleCube2 = new GameObject(cubeVertices2);

    std::vector<float> vertices = {
        // Positions      // UVs
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,  // Vertex 1
        0.5f, -0.5f, 0.0f, 1.0f, 1.0f,  // Vertex 2
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f,  // Vertex 3

        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,  // Vertex 1 (Repeated)
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f,  // Vertex 3 (Repeated)
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f   // Vertex 4
    };
    GameObject* rectangle = new GameObject(vertices, 0);


    Model lija("res/low-poly-fox.obj");
    Model steeringWheelModel("res/Isuzu NKR Steering Wheels.obj");

    Shader phongShader("phong.vert", "phong.frag");
    Shader hudShader("hud.vert", "hud.frag");
    Shader dShader("2dSahder.vert", "2dShader.frag");


    phongShader.use();
    glm::mat4 model = glm::mat4(1.0f);
    phongShader.setMat4("uModel", model);

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    phongShader.setMat4("uView", view);

    glm::mat4 projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 200.0f);
    phongShader.setMat4("uProjection", projectionP);

    phongShader.setVec3("uViewPos", 0.0, 0.0, 5.0);

    phongShader.setVec3("uDirLight.Position", 0.0, 200, 0.0);
    phongShader.setVec3("uDirLight.Direction", 0.0, -1.0, 0.0);
    phongShader.setVec3("uDirLight.Ka", glm::vec3(0.2));
    phongShader.setVec3("uDirLight.Kd", glm::vec3(0.2));
    phongShader.setVec3("uDirLight.Ks", glm::vec3(10.f));

    /*phongShader.setVec3("uSpotlights[0].Position", 0.0, 10.0, 0.0);
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
    phongShader.setFloat("uSpotlights[1].Kq", 0.032f);*/

    /*phongShader.setVec3("uPointLights[0].Position", glm::vec3(-500));
    phongShader.setVec3("uPointLights[0].Ka", glm::vec3(230.0 / 255 / 0.1, 92.0 / 255 / 0.1, 0.0f));
    phongShader.setVec3("uPointLights[0].Kd", glm::vec3(230.0 / 255 / 50, 92.0 / 255 / 50, 0.0f));
    phongShader.setVec3("uPointLights[0].Ks", glm::vec3(1.0f));
    phongShader.setFloat("uPointLights[0].Kc", 1.5f);
    phongShader.setFloat("uPointLights[0].Kl", 1.0f);
    phongShader.setFloat("uPointLights[0].Kq", 0.272f);*/

    unsigned texture = Model::textureFromFile("res/kurac.png");
    unsigned hudTex = Model::textureFromFile("res/hudTex.png");
    unsigned kockaDif = Model::textureFromFile("res/container_diffuse.png");
    unsigned kockaSpec = Model::textureFromFile("res/container_specular.png");
    unsigned buildingDif = Model::textureFromFile("res/zgrada.png");
    unsigned buildingSpec = Model::textureFromFile("res/zgrada_spec.png");
    unsigned asphalt = Model::textureFromFile("res/cracked-asphalt-texture.jpg");
    unsigned tachometer = Model::textureFromFile("res/speedometer.png");

    phongShader.setInt("uMaterial.Kd", 0);
    phongShader.setInt("uMaterial.Ks", 1);
    phongShader.setFloat("uMaterial.Shininess", 0.25 * 128);

    phongShader.setBool("uTransp", false);

    Params params;
    glfwSetWindowUserPointer(window, &params);

    glm::mat4 model2 = glm::mat4(1.0f);
    glm::mat4 m(1.0f);
    float currentRot = 0;
    float FrameStartTime = 0;
    float FrameEndTime = 0;

    glClearColor(0.2, 0.2, 0.6, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    while (!glfwWindowShouldClose(window))
    {
        FrameStartTime = glfwGetTime();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwPollEvents();
        //Loop

        phongShader.use();

        //Camera
        HandleInput(&params);

        float radius = 1.0f;
        float angle = glm::radians(90-params.carRot);

        float cameraX = params.carOffset.x + radius * cos(angle);
        float cameraZ = params.carOffset.z + radius * sin(angle);

        // Update camera front vector
        glm::vec3 front;
        front.x = cos(glm::radians(90 - params.carRot));
        front.y = 0.0f;
        front.z = sin(glm::radians(90 - params.carRot));
        front = glm::normalize(front);

        params.carForward = front;

        glm::mat4 view = glm::lookAt(glm::vec3(cameraX, 2.3, cameraZ),
            glm::vec3(cameraX, 2.3, cameraZ) + front,
            params.cameraUp);

        phongShader.setMat4("uView", view);
        phongShader.setVec3("uViewPos", glm::vec3(cameraX, 2.3, cameraZ));
        phongShader.setMat4("uProjection", projectionP);

        //FPS
        /*view = glm::lookAt(params.position,
            params.position + params.cameraFront,
            params.cameraUp);

        phongShader.setMat4("uView", view);
        phongShader.setVec3("uViewPos", params.position);*/

        //2D Instruemtns
        //glm::vec3(1.0, 0.85, 3.89)
        dShader.use();
        dShader.setMat4("uView", view);
        dShader.setMat4("uProjection", projectionP);

        

        //TachmoIndicator
        float rotation = (300 * glm::abs(params.velocity) / 60.0);

        m = glm::translate(glm::mat4(1.0), params.carOffset);
        m = glm::rotate(m, glm::radians(params.carRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.carOffset);

        m = glm::translate(m, glm::vec3(0.8, 0.85, 3.88) + params.carOffset);
        m = glm::rotate(m, glm::radians(30.f + rotation), glm::vec3(0.0, 0.0, 1.0));
        m = glm::translate(m, -glm::vec3(0.8, 0.85, 3.88) - params.carOffset);

        m = glm::translate(m, glm::vec3(0.8, 0.77, 3.88));
        m = glm::translate(m, params.carOffset);
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.02, 0.2, 1.0));
        dShader.setMat4("uModel", m);
        rectangle->Render(&dShader, 0,0,1);

        //EngineLight
        m = glm::translate(glm::mat4(1.0), params.carOffset);
        m = glm::rotate(m, glm::radians(params.carRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.carOffset);

        m = glm::translate(m, glm::vec3(-0.8, 0.85, 3.89));
        m = glm::translate(m, params.carOffset);
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.2));
        dShader.setMat4("uModel", m);
        rectangle->Render(&dShader, 1,1,0);

        glm::vec3 engineLightPosition = glm::vec3(m[3]);

        phongShader.use();
        if (!params.engineBroken) {
            glm::vec3 zeroVec = glm::vec3(0.0f);
            phongShader.setVec3("uPointLights[0].Ka", zeroVec);
            phongShader.setVec3("uPointLights[0].Kd", zeroVec);
            phongShader.setVec3("uPointLights[0].Ks", zeroVec);
        }
        else
        {
            phongShader.setVec3("uPointLights[0].Ka", glm::vec3(0.1, 0.1, 0));
            phongShader.setVec3("uPointLights[0].Kd", glm::vec3(4, 4, 0));
            phongShader.setVec3("uPointLights[0].Ks", glm::vec3(1.0f,1,0));
        }
        phongShader.setVec3("uPointLights[0].Position", engineLightPosition);
        phongShader.setFloat("uPointLights[0].Kc", 0.5f);
        phongShader.setFloat("uPointLights[0].Kl", 0.2f);
        phongShader.setFloat("uPointLights[0].Kq", 10.0f);
        dShader.use();

        //BatteryLight
        m = glm::translate(glm::mat4(1.0), params.carOffset);
        m = glm::rotate(m, glm::radians(params.carRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.carOffset);

        m = glm::translate(m, glm::vec3(-1.1, 0.85, 3.89));
        m = glm::translate(m, params.carOffset);
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.2));
        dShader.setMat4("uModel", m);
        rectangle->Render(&dShader, 1, 0, 0);

        glm::vec3 batteryLightPosition = glm::vec3(m[3]);

        phongShader.use();
        if (!params.bateryBroken) {
            glm::vec3 zeroVec = glm::vec3(0.0f);
            phongShader.setVec3("uPointLights[1].Ka", zeroVec);
            phongShader.setVec3("uPointLights[1].Kd", zeroVec);
            phongShader.setVec3("uPointLights[1].Ks", zeroVec);
        }
        else
        {
            phongShader.setVec3("uPointLights[1].Ka", glm::vec3(0.1, 0, 0));
            phongShader.setVec3("uPointLights[1].Kd", glm::vec3(4, 0, 0));
            phongShader.setVec3("uPointLights[1].Ks", glm::vec3(1.0f,0,0));
        }
        phongShader.setVec3("uPointLights[1].Position", batteryLightPosition);
        phongShader.setFloat("uPointLights[1].Kc", 0.5f);
        phongShader.setFloat("uPointLights[1].Kl", 0.2f);
        phongShader.setFloat("uPointLights[1].Kq", 10.0f);
        dShader.use();


        //------------

        phongShader.use();

        //NightVision / Normal vision
        if (!params.nightVision) {
            phongShader.setVec3("uDirLight.Position", 0.0, 20, 0.0);
            phongShader.setVec3("uDirLight.Direction", 0.1, -5, 0.1);
            phongShader.setVec3("uDirLight.Ka", glm::vec3(0.2));
            phongShader.setVec3("uDirLight.Kd", glm::vec3(0.2));
            phongShader.setVec3("uDirLight.Ks", glm::vec3(10.f));
        }
        else
        {
            phongShader.setVec3("uDirLight.Position", 0.0, 20, 0.0);
            phongShader.setVec3("uDirLight.Direction", 0.1, -5, 0.1);
            phongShader.setVec3("uDirLight.Ka", glm::vec3(0,0.5,0));
            phongShader.setVec3("uDirLight.Kd", glm::vec3(0, 0.6, 0));
            phongShader.setVec3("uDirLight.Ks", glm::vec3(10.f));
        }
        

        //Road
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -1.0, 0.0));
        m = glm::scale(m, glm::vec3(70.0, 1.0, 2000));
        phongShader.setMat4("uModel", m);
        simpleCube2->Render(&phongShader, asphalt, asphalt);

        //Ground
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -1.1, 120.0));
        m = glm::scale(m, glm::vec3(300.0, 1.0, 2300.));
        phongShader.setMat4("uModel", m);
        simpleCube2->Render(&phongShader, 0,1,0);

        //CarBase
        m = glm::translate(glm::mat4(1.0), params.carOffset);
        m = glm::rotate(m, glm::radians(params.carRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.carOffset);

        m = glm::translate(m, glm::vec3(0.0 , 0.0, 0.0));
        m = glm::translate(m, params.carOffset);
        m = glm::scale(m, glm::vec3(6.0, 1.0, 12.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.1, 0.1, 0.1);

        glm::vec3 frontSidePosition = glm::vec3(m[3]) + front*6.f;

        glm::vec3 arbitraryVector(0.0f, 1.0f, 0.0f);
        glm::vec3 rightVector = glm::cross(front, arbitraryVector);
        rightVector = glm::normalize(rightVector);

        glm::vec3 headlight1Pos = frontSidePosition + rightVector * 2.6f;
        glm::vec3 headlight2Pos = frontSidePosition - rightVector * 2.6f;

        glm::vec3 lightInt = glm::vec3(0);
        glm::vec3 lightIntA = glm::vec3(0);
        glm::vec3 lightIntS = glm::vec3(0);
        if (params.headlights) {
            lightInt = glm::vec3(3.0f, 3.0f, 3.0f);
            lightIntA = glm::vec3(0.5);
            lightIntS = glm::vec3(60.0f);
        }

        phongShader.setVec3("uSpotlights[0].Position", headlight1Pos);
        phongShader.setVec3("uSpotlights[0].Direction", front);
        phongShader.setVec3("uSpotlights[0].Ka", lightIntA);
        phongShader.setVec3("uSpotlights[0].Kd", lightInt);
        phongShader.setVec3("uSpotlights[0].Ks", lightIntS);
        phongShader.setFloat("uSpotlights[0].InnerCutOff", glm::cos(glm::radians(15.0f)));
        phongShader.setFloat("uSpotlights[0].OuterCutOff", glm::cos(glm::radians(25.0f)));
        phongShader.setFloat("uSpotlights[0].Kc", 1.0);
        phongShader.setFloat("uSpotlights[0].Kl", 0.09);
        phongShader.setFloat("uSpotlights[0].Kq", 0.0f);

        phongShader.setVec3("uSpotlights[1].Position", headlight2Pos);
        phongShader.setVec3("uSpotlights[1].Direction", front);
        phongShader.setVec3("uSpotlights[1].Ka", lightIntA);
        phongShader.setVec3("uSpotlights[1].Kd", lightInt);
        phongShader.setVec3("uSpotlights[1].Ks", lightIntS);
        phongShader.setFloat("uSpotlights[1].InnerCutOff", glm::cos(glm::radians(15.0f)));
        phongShader.setFloat("uSpotlights[1].OuterCutOff", glm::cos(glm::radians(25.0f)));
        phongShader.setFloat("uSpotlights[1].Kc", 1.0);
        phongShader.setFloat("uSpotlights[1].Kl", 0.09f);
        phongShader.setFloat("uSpotlights[1].Kq", 0.0f);

        //CarSides
        m = glm::translate(glm::mat4(1.0), params.carOffset);
        m = glm::rotate(m, glm::radians(params.carRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.carOffset);

        m = glm::translate(m, glm::vec3(-2.5, 1.5, 1.5));
        m = glm::translate(m, params.carOffset);
        m = glm::rotate(m, glm::radians(-30.f), glm::vec3(1.0, 0.0, 0.0));
        m = glm::scale(m, glm::vec3(1.0, 5.0, 5.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 0, 0);

        m = glm::translate(glm::mat4(1.0), params.carOffset);
        m = glm::rotate(m, glm::radians(params.carRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.carOffset);

        m = glm::translate(m, glm::vec3(2.5, 1.5, 1.5));
        m = glm::translate(m, params.carOffset);
        m = glm::rotate(m, glm::radians(-30.f), glm::vec3(1.0, 0.0, 0.0));
        m = glm::scale(m, glm::vec3(1.0, 5.0, 5.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 0, 0);

        m = glm::translate(glm::mat4(1.0), params.carOffset);
        m = glm::rotate(m, glm::radians(params.carRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.carOffset);

        m = glm::translate(m, glm::vec3(0.0, 2.0, -4.5));
        m = glm::translate(m, params.carOffset);
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::rotate(m, glm::radians(20.f), glm::vec3(0.0, 0.0, 1.0));
        m = glm::scale(m, glm::vec3(1.0, 5.0, 6.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 0, 0);

        //CarTop
        m = glm::translate(glm::mat4(1.0), params.carOffset);
        m = glm::rotate(m, glm::radians(params.carRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.carOffset);

        m = glm::translate(m, glm::vec3(0.0, 4.5, -1.0));
        m = glm::translate(m, params.carOffset);
        m = glm::scale(m, glm::vec3(6.0, 1.0, 8.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 0, 0);

        //Instrument
        m = glm::translate(glm::mat4(1.0), params.carOffset);
        m = glm::rotate(m, glm::radians(params.carRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.carOffset);

        m = glm::translate(m, glm::vec3(0.0, 0.7, 4.0));
        m = glm::translate(m, params.carOffset);
        m = glm::scale(m, glm::vec3(4.0, 1.0, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.42, 0.42, 0.42);


        //Steering wheel
        m = glm::translate(glm::mat4(1.0), params.carOffset);
        m = glm::rotate(m, glm::radians(params.carRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.carOffset);

        m = glm::translate(m, glm::vec3(0.0, 1.2, 3.8));
        m = glm::translate(m, params.carOffset);
        m = glm::rotate(m, glm::radians(-80.f), glm::vec3(1.0, 0.0, 0.0));
        m = glm::rotate(m, glm::radians(params.carRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.2));
        phongShader.setMat4("uModel", m);
        steeringWheelModel.Draw(phongShader);

        //Building
        DrawBuildings(phongShader, simpleCube, buildingDif, buildingSpec);

        //Glass
        m = glm::translate(glm::mat4(1.0), params.carOffset);
        m = glm::rotate(m, glm::radians(params.carRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.carOffset);

        m = glm::translate(m, glm::vec3(0.0, 2, 4.0));
        m = glm::translate(m, params.carOffset);
        m = glm::rotate(m, glm::radians(-30.f), glm::vec3(1.0, 0.0, 0.0));
        m = glm::scale(m, glm::vec3(4.2, 6.0, 0.1));
        phongShader.setMat4("uModel", m);
        phongShader.setBool("uTransp", true);
        simpleCube->Render(&phongShader, 0.03, 0.1, 0.95);
        phongShader.setBool("uTransp", false);



        //hud
        DrawHud(hudShader, hudTex);

        //Tachometar
        dShader.use();
        m = glm::translate(glm::mat4(1.0), params.carOffset);
        m = glm::rotate(m, glm::radians(params.carRot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.carOffset);

        m = glm::translate(m, glm::vec3(0.8, 0.85, 3.89));
        m = glm::translate(m, params.carOffset);
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.4));
        dShader.setMat4("uModel", m);
        rectangle->Render(&dShader, tachometer);

        //end
        glfwSwapBuffers(window);
        glUseProgram(0);
        FrameEndTime = glfwGetTime();
        params.dt = FrameEndTime - FrameStartTime;
    }
    glfwTerminate();
    return 0;
}


