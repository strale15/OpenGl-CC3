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

#include <stdlib.h>
#include <time.h> 
#include <string>
#include <cmath>

#define NUMBER_OF_HELICOPTERS 10
#define NUMBER_OF_LOW_HELICOPTERS 5
#define NUMBER_OF_CLODS 50

const unsigned int wWidth = 1920;
const unsigned int wHeight = 1080;

bool firstMouse = true;
double lastX;
double lastY;

struct Cloud {
    float rotation = 0;
    glm::vec3 position = glm::vec3(0.0, 3.0, 0.0);
    float scaleX = 1;
    float scaleZ = 1;
    float alpha = 0.5;

};

Cloud clouds[NUMBER_OF_CLODS];

struct Helicopter {
    glm::vec3 position = glm::vec3(0.0, -5.0, 0.0);
    bool isAlive = true;
    float speed = 0.6;
    bool isLowFlight = false;
    float angle = 0;
};

Helicopter targets[NUMBER_OF_HELICOPTERS + NUMBER_OF_LOW_HELICOPTERS];

struct Params {
    float dt = 0;
    bool isFps = true;

    glm::vec3 cameraFront = glm::vec3(0.0, 0.0, 1.0);
    glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);
    glm::vec3 position = glm::vec3(0.0, 11.5, -13.5);

    glm::vec3 objPos = glm::vec3(0.0, 0.0, 0.0);

    double camYaw = 90;
    double camPitch = 0;

    bool wDown = false;
    bool sDown = false;
    bool aDown = false;
    bool dDown = false;

    bool spaceDown = false;
    bool shiftDown = false;

    //Drone
    bool droneUp = false;
    bool droneDown = false;
    bool droneLeft = false;
    bool droneRight = false;
    bool droneForward = false;
    bool droneBackward = false;

    bool isMapActive = true;
    bool numberOfDrones = 7;
    bool isDroneAlive = true;
    glm::vec3 dronePosition = glm::vec3(0.0, 1.0, -5.0);
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

static void GenerateTargets() {
    for (int i = 0; i < NUMBER_OF_HELICOPTERS; i++) {
        targets[i].isLowFlight = false;

        float randomCoord = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 30.0) - 15.0;
        float randomCoorY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 10.0) + 4;
        int random = rand() % 2;
        if (random == 0) {
            targets[i].position.x = randomCoord;
            targets[i].position.z = 15;
            targets[i].position.y = randomCoorY;
        }
        else if (random == 1) {
            targets[i].position.z = glm::abs(randomCoord);
            targets[i].position.x = 15;
            targets[i].position.y = randomCoorY;
        }
        else {
            targets[i].position.z = glm::abs(randomCoord);
            targets[i].position.x = -15;
            targets[i].position.y = randomCoorY;
        }

        glm::vec3 cityCenter = glm::vec3(0, targets[i].position.y, 5);
        glm::vec3 forward = glm::vec3(-1, 0, 0);
        float dotProduct = glm::dot(glm::normalize(cityCenter- targets[i].position), forward);
        dotProduct = glm::clamp(dotProduct, -1.f, 1.f);
        float angleRadians = std::acos(dotProduct);
        float angleDegrees = glm::degrees(angleRadians);
        if (targets[i].position.z < 5) {
            angleDegrees *= -1;
        }
        targets[i].angle = angleDegrees;

        cout << "Target " << i << " Pos " << targets[i].position.x << " " << targets[i].position.y << " " << targets[i].position.z << endl;
        
    }

    for (int i = NUMBER_OF_HELICOPTERS; i < NUMBER_OF_LOW_HELICOPTERS+ NUMBER_OF_HELICOPTERS; i++) {
        targets[i].isLowFlight = true;

        float randomCoord = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 30.0) - 15.0;
        int random = rand() % 3;
        if (random == 0) {
            targets[i].position.x = randomCoord;
            targets[i].position.z = 15;
            targets[i].position.y = 2.0;
        }
        else if(random == 1) {
            targets[i].position.z = glm::abs(randomCoord);
            targets[i].position.x = 15;
            targets[i].position.y = 2.0;
        }
        else {
            targets[i].position.z = glm::abs(randomCoord);
            targets[i].position.x = -15;
            targets[i].position.y = 2.0;
        }

        glm::vec3 cityCenter = glm::vec3(0, targets[i].position.y, 5);
        glm::vec3 forward = glm::vec3(-1, 0, 0);
        float dotProduct = glm::dot(glm::normalize(cityCenter - targets[i].position), glm::normalize(forward));
        dotProduct = glm::clamp(dotProduct, -1.f, 1.f);
        float angleRadians = std::acos(dotProduct);
        float angleDegrees = glm::degrees(angleRadians);
        if (targets[i].position.z < 5) {
            angleDegrees *= -1;
        }
        targets[i].angle = angleDegrees;

        cout << "TargetLow " << i << " Pos " << targets[i].position.x << " " << targets[i].position.y << " " << targets[i].position.z << endl;

    }
}

static void RenderAliveHelicopters(Shader& shader, Model targetModel) {
    glm::mat4 m = glm::mat4(1.0);
    for (int i = 0; i < NUMBER_OF_HELICOPTERS + NUMBER_OF_LOW_HELICOPTERS; i++) {
        float coef = 0;
        if (targets[i].isAlive) {
            m = glm::translate(glm::mat4(1.0), targets[i].position);
            m = glm::rotate(m, glm::radians(-targets[i].angle), glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(0.002));
            shader.setMat4("uModel", m);
            shader.setBool("isColor", true);
            if (targets[i].isLowFlight) {
                shader.setVec3("uColor", glm::vec3(1, 0, 0));
            }
            else {
                shader.setVec3("uColor", glm::vec3(0.8, 0.8, 0));
            }
            targetModel.Draw(shader);
            shader.setBool("isColor", false);

            if(!targets[i].isLowFlight)
                coef = 1;

        }
        if (i < NUMBER_OF_HELICOPTERS) {
            std::string lightUniform = "uPointLights[";
            lightUniform.append(to_string(i));
            lightUniform.append("].");
            glm::vec3 pos = targets[i].position;
            pos.y += 0.3;
            shader.setVec3(lightUniform + "Position", pos);
            shader.setVec3(lightUniform + "Ka", glm::vec3(0.1f)* coef);
            shader.setVec3(lightUniform + "Kd", glm::vec3(1.f,1.0f,0)* coef);
            shader.setVec3(lightUniform + "Ks", glm::vec3(1.0f,1.0f,0)* coef);
            shader.setFloat(lightUniform + "Kc", 1.5f);
            shader.setFloat(lightUniform + "Kl", 3.0f);
            shader.setFloat(lightUniform + "Kq", 2.272f);
        }
    }
}

static void GenerateCloudPositions() {
    for (int i = 0; i < NUMBER_OF_CLODS; i++) {
        float randomCoordX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 30.0) - 15.0;
        float randomCoordZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 30.0) - 15.0;
        float randomCoordY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 2.0) + 3;
        float randomRot = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 80.0) - 40;
        float randomScaleX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 1.4) + 0.2;
        float randomScaleZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 1.4) + 0.2;
        float randomAlpha = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 0.3) + 0.4;

        clouds[i].position = glm::vec3(randomCoordX, randomCoordY, randomCoordZ);
        clouds[i].rotation = randomRot;
        clouds[i].scaleX = randomScaleX;
        clouds[i].scaleZ = randomScaleZ;
        clouds[i].alpha = randomAlpha;
    }
}

static void UpdateTargetsPos(float dt) {
    for (int i = 0; i < NUMBER_OF_HELICOPTERS + NUMBER_OF_LOW_HELICOPTERS; i++) {
        if (!targets[i].isAlive) {
            continue;
        }
        glm::vec3 cityCenter = glm::vec3(0, targets[i].position.y, 5);
        if (glm::distance(targets[i].position, cityCenter) <= 0.5) {
            continue;
        }

        glm::vec3 direction = cityCenter - targets[i].position;
        float lowT = 1;
        if (targets[i].isLowFlight) {
            lowT = 0.33;
        }
        targets[i].position += glm::normalize(direction) * dt * targets[i].speed * lowT;
    }
}

static void CheckIfDroneCrashed(Params* params) {
    if (!params->isDroneAlive) {
        return;
    }

    if (params->dronePosition.y < 0.5 || params->dronePosition.y > 20) {
        cout << "Previsoko/Prenisko" << endl;
        params->isDroneAlive = false;
        return;
    }
    if (glm::abs(params->dronePosition.x) > 15 || glm::abs(params->dronePosition.z) > 15) {
        cout << "Izleteo" << endl;
        params->isDroneAlive = false;
        return;
    }

    for (int i = 0; i < NUMBER_OF_HELICOPTERS + NUMBER_OF_LOW_HELICOPTERS; i++) {
        if (targets[i].isAlive) {
            if (glm::distance(targets[i].position, params->dronePosition) <= 1.2) {
                targets[i].isAlive = false;
                params->isDroneAlive = false;
                cout << "Skuco u " << i << endl;
                return;
            }
        }
    }
}

std::vector<float> generateCircleVertices(float radius, int numSegments) {
    std::vector<float> vertices;

    for (int i = 0; i < numSegments; ++i) {
        float theta1 = 2.0f * M_PI * i / numSegments;
        float theta2 = 2.0f * M_PI * (i + 1) / numSegments;

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

    //Drone
    float droneSpeed = targets[0].speed *2;
    if (params->droneForward)
    {
        params->dronePosition.z += droneSpeed * params->dt;
    }
    if (params->droneBackward)
    {
        params->dronePosition.z -= droneSpeed * params->dt;
    }
    if (params->droneLeft)
    {
        params->dronePosition.x += droneSpeed * params->dt;
    }
    if (params->droneRight)
    {
        params->dronePosition.x -= droneSpeed * params->dt;
    }
    if (params->droneUp)
    {
        params->dronePosition.y += droneSpeed * params->dt;
    }
    if (params->droneDown)
    {
        params->dronePosition.y -= droneSpeed * params->dt;
    }
}

static void CursosPosCallback(GLFWwindow* window, double xPos, double yPos) {
    Params* params = (Params*)glfwGetWindowUserPointer(window);

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

static void KeyCallback2(GLFWwindow* window, int key, int scancode, int action, int mode) {
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
            params->droneForward = true;
        }
        else if (action == GLFW_RELEASE) {
            params->droneForward = false;
        }
    }
    if (key == GLFW_KEY_DOWN) {
        if (action == GLFW_PRESS) {
            params->droneBackward = true;
        }
        else if (action == GLFW_RELEASE) {
            params->droneBackward = false;
        }
    }
    if (key == GLFW_KEY_LEFT) {
        if (action == GLFW_PRESS) {
            params->droneLeft = true;
        }
        else if (action == GLFW_RELEASE) {
            params->droneLeft = false;
        }
    }
    if (key == GLFW_KEY_RIGHT) {
        if (action == GLFW_PRESS) {
            params->droneRight = true;
        }
        else if (action == GLFW_RELEASE) {
            params->droneRight = false;
        }
    }
    if (key == GLFW_KEY_Q) {
        if (action == GLFW_PRESS) {
            params->droneUp = true;
        }
        else if (action == GLFW_RELEASE) {
            params->droneUp = false;
        }
    }
    if (key == GLFW_KEY_E) {
        if (action == GLFW_PRESS) {
            params->droneDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->droneDown = false;
        }
    }

    //Map
    if (key == GLFW_KEY_F) {
        if (action == GLFW_PRESS) {
            params->isMapActive = !params->isMapActive;
        }
    }

    if (key == GLFW_KEY_L) {
        if (action == GLFW_PRESS && !params->isDroneAlive) {
            params->dronePosition = glm::vec3(0.0, 1.0, -5.0);
            params->numberOfDrones -= 1;
            params->isDroneAlive = true;
        }
    }
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
    glfwSetKeyCallback(window, KeyCallback2);
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

    std::vector<float> circleVert = generateCircleVertices(1, 64);
    GameObject* circle = new GameObject(circleVert, true);

    Model droneObj("res/Drone_LP.obj");
    Model copterObj("res/Copter_2.obj");

    Shader phongShader("phong.vert", "phong.frag");
    Shader hudShader("hud.vert", "hud.frag");
    Shader twoD("twoD.vert", "twoD.frag");

    phongShader.use();

    glm::mat4 view;
    glm::mat4 projectionP;

    phongShader.setVec3("uDirLight.Position", glm::vec3(0.0, 15.0f, 0.0f));
    phongShader.setVec3("uDirLight.Direction", 0.1, -1, 0.1);
    phongShader.setVec3("uDirLight.Ka", glm::vec3(0.11));
    phongShader.setVec3("uDirLight.Kd", glm::vec3(0.2));
    phongShader.setVec3("uDirLight.Ks", glm::vec3(1.0, 1.0, 1.0));

    phongShader.setVec3("uPointLights[0].Position", glm::vec3(-99999));
    phongShader.setVec3("uPointLights[0].Ka", glm::vec3(0.2f));
    phongShader.setVec3("uPointLights[0].Kd", glm::vec3(0.6f));
    phongShader.setVec3("uPointLights[0].Ks", glm::vec3(1.0f));
    phongShader.setFloat("uPointLights[0].Kc", 1.5f);
    phongShader.setFloat("uPointLights[0].Kl", 1.0f);
    phongShader.setFloat("uPointLights[0].Kq", 0.272f);

    unsigned hudTex = Model::textureFromFile("res/hudTex.png");
    unsigned kockaDif = Model::textureFromFile("res/container_diffuse.png");
    unsigned kockaSpec = Model::textureFromFile("res/container_specular.png");
    unsigned mapTex = Model::textureFromFile("res/map1.png");
    unsigned mapflipTex = Model::textureFromFile("res/map1Flip.png");
    unsigned mapSpecTex = Model::textureFromFile("res/map1Spec.png");
    unsigned modelSpec = Model::textureFromFile("res/Part2_Metallic.png");

    phongShader.setInt("uMaterial.Kd", 0);
    phongShader.setInt("uMaterial.Ks", 1);
    phongShader.setFloat("uMaterial.Shininess", 0.5 * 128);

    glm::mat4 m(1.0f);
    float currentRot = 0;
    float FrameStartTime = 0;
    float FrameEndTime = 0;

    Params params;
    glfwSetWindowUserPointer(window, &params);

    glClearColor(0, 0, 0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    GenerateTargets();
    GenerateCloudPositions();
    while (!glfwWindowShouldClose(window))
    {
        FrameStartTime = glfwGetTime();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        CheckIfDroneCrashed(&params);
        UpdateTargetsPos(params.dt);

        //Loop
        phongShader.use();
        HandleInput(&params);

        //Camera
        view = glm::lookAt(params.position, params.position + params.cameraFront, params.cameraUp);
        projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);

        phongShader.setMat4("uView", view);
        phongShader.setMat4("uProjection", projectionP);
        phongShader.setVec3("uViewPos", params.position);

        //SCENE
        //------------------------------------------------------------------------------------------------------------
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
        m = glm::rotate(m, glm::radians(0.f), glm::vec3(1.0, 0.0, 0.0));
        m = glm::scale(m, glm::vec3(30,1.0,30));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, mapTex, mapSpecTex);

        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 5.5, -13.5));
        m = glm::scale(m, glm::vec3(3, 10.0, 3));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1,0,0);

        if (params.isDroneAlive) {
            m = glm::translate(glm::mat4(1.0), params.dronePosition);
            m = glm::scale(m, glm::vec3(0.2));
            phongShader.setMat4("uModel", m);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, modelSpec);
            droneObj.Draw(phongShader);
        }

        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 20.0, 5));
        m = glm::scale(m, glm::vec3(60, 1.0, 60));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 1);


        float time = glfwGetTime() * 15.f;
        glm::vec3 dir = glm::vec3(sin(glm::radians(time)), 1.0, cos(glm::radians(time)));

        phongShader.setVec3("uSpotlights[0].Position", glm::vec3(0,1,5));
        phongShader.setVec3("uSpotlights[0].Direction", dir);
        phongShader.setVec3("uSpotlights[0].Ka", 0.0, 0.0, 0.0);
        phongShader.setVec3("uSpotlights[0].Kd", glm::vec3(2.f));
        phongShader.setVec3("uSpotlights[0].Ks", glm::vec3(1.0));
        phongShader.setFloat("uSpotlights[0].InnerCutOff", glm::cos(glm::radians(35.0f)));
        phongShader.setFloat("uSpotlights[0].OuterCutOff", glm::cos(glm::radians(45.0f)));
        phongShader.setFloat("uSpotlights[0].Kc", 1.0);
        phongShader.setFloat("uSpotlights[0].Kl", 0.072f);
        phongShader.setFloat("uSpotlights[0].Kq", 0.012f);

        RenderAliveHelicopters(phongShader, copterObj);

        for (int i = 0; i < NUMBER_OF_CLODS; i++) {
            m = glm::translate(glm::mat4(1.0), clouds[i].position);
            m = glm::rotate(m, glm::radians(clouds[i].rotation), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(clouds[i].scaleX, 0.2, clouds[i].scaleZ));
            phongShader.setMat4("uModel", m);
            phongShader.setBool("uTransp", true);
            phongShader.setFloat("uAlpha", clouds[i].alpha);
            simpleCube->Render(&phongShader, 1, 1, 1);
            phongShader.setBool("uTransp", false);
        }

        //------------------------------------------------------------------------------------------------------------

        //2D
        twoD.use();
        twoD.setMat4("uView", view);
        twoD.setMat4("uProjection", projectionP);

        float screenHeight = 11;
        float screenRot = 75;
        glm::vec3 mapOffset = glm::vec3(0);
        glm::vec3 posOnMap = glm::vec3(0);

        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, screenHeight, -12.0));
        m = glm::translate(m, mapOffset);
        m = glm::rotate(m, glm::radians(screenRot), glm::vec3(1.0, 0.0, 0.0));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.5));
        twoD.setMat4("uModel", m);
        rectangle->Render(&twoD, mapflipTex);

        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, screenHeight+0.01, -12.0));
        m = glm::translate(m, mapOffset);
        m = glm::translate(m, posOnMap);
        m = glm::rotate(m, glm::radians(screenRot), glm::vec3(1.0, 0.0, 0.0));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.01));
        twoD.setMat4("uModel", m);
        circle->Render(&twoD, 1,0,0);



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


