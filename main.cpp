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

#define NUMBER_OF_HELICOPTERS 5
#define NUMBER_OF_LOW_HELICOPTERS 2
#define NUMBER_OF_CLODS 55

const unsigned int wWidth = 1920;
const unsigned int wHeight = 1080;

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
    glm::vec3 lastKnowsPosition = glm::vec3(0.0, -5.0, 0.0);
    bool isAlive = false;
    bool isSpawned = false;
    float speed = 0.15;
    bool isLowFlight = false;
    float angle = 0;
    float spawnTime = 0;
};

Helicopter targets[NUMBER_OF_HELICOPTERS + NUMBER_OF_LOW_HELICOPTERS];

struct Params {
    float dt = 0;
    glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

    //Drone
    bool droneUp = false;
    bool droneDown = false;
    bool droneLeft = false;
    bool droneRight = false;
    bool droneForward = false;
    bool droneBackward = false;

    bool isMapActive = true;
    int numberOfDrones = 7;
    bool isDroneAlive = false;
    glm::vec3 dronePosition = glm::vec3(0.0, 7.5, -10.0);
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
        float randomTime = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 10+13*i) + 2+7*i;
        int random = rand() % 2;
        if (random == 0) {
            targets[i].position.x = randomCoord;
            targets[i].position.z = 15;
            targets[i].position.y = randomCoorY;
        }
        else if (random == 1) {
            targets[i].position.z = randomCoord;
            targets[i].position.x = 15;
            targets[i].position.y = randomCoorY;
        }
        else {
            targets[i].position.z = randomCoord;
            targets[i].position.x = -15;
            targets[i].position.y = randomCoorY;
        }

        targets[i].spawnTime = randomTime;

        //Racunanje ugla da bi letelica bila okrenuta ka centru
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

        //cout << "Target " << i << " Pos " << targets[i].position.x << " " << targets[i].position.y << " " << targets[i].position.z << endl;
        
    }

    //Sve ovo isto sto i gore ali za niske letelice
    for (int i = NUMBER_OF_HELICOPTERS; i < NUMBER_OF_LOW_HELICOPTERS+ NUMBER_OF_HELICOPTERS; i++) {
        targets[i].isLowFlight = true;

        float randomCoord = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 30.0) - 15.0;
        float randomTime = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 10+4*i) + 8+3*i;

        int random = rand() % 3;
        if (random == 0) {
            targets[i].position.x = randomCoord;
            targets[i].position.z = 15;
            targets[i].position.y = 2.0;
        }
        else if(random == 1) {
            targets[i].position.z = randomCoord;
            targets[i].position.x = 15;
            targets[i].position.y = 2.0;
        }
        else {
            targets[i].position.z = randomCoord;
            targets[i].position.x = -15;
            targets[i].position.y = 2.0;
        }

        targets[i].spawnTime = randomTime;

        //Racunanje ugla da bi letelica bila okrenuta ka centru
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
            m = glm::scale(m, glm::vec3(0.003));
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
            shader.setVec3(lightUniform + "Ka", glm::vec3(0.12f)* coef);
            shader.setVec3(lightUniform + "Kd", glm::vec3(1.5f,1.50f,0)* coef);
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
        if (glm::distance(targets[i].position, cityCenter) <= 0.1) {
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
            if (glm::distance(targets[i].position, params->dronePosition) < 0.5) {
                targets[i].isAlive = false;
                params->isDroneAlive = false;
                cout << "Skuco u " << i << endl;
                return;
            }
        }
    }
}

static void SpawnTargets(Params* params) {
    float elapsedTime = glfwGetTime();

    for (int i = 0; i < NUMBER_OF_HELICOPTERS + NUMBER_OF_LOW_HELICOPTERS; i++) {
        if (elapsedTime > targets[i].spawnTime && !targets[i].isSpawned && !targets[i].isAlive) {
            targets[i].isAlive = true;
            targets[i].isSpawned = true;
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
    //Drone
    float droneSpeed = targets[0].speed * 5;
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
        params->dronePosition.y += droneSpeed * params->dt * 2;
    }
    if (params->droneDown)
    {
        params->dronePosition.y -= droneSpeed * params->dt * 2;
    }


}

static void KeyCallback2(GLFWwindow* window, int key, int scancode, int action, int mode) {
    Params* params = (Params*)glfwGetWindowUserPointer(window);

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

    if (key == GLFW_KEY_SPACE) {
        if (action == GLFW_PRESS && !params->isDroneAlive && params->numberOfDrones > 0) {
            params->dronePosition = glm::vec3(0.0, 7.5, -10.0);
            params->numberOfDrones -= 1;
            params->isDroneAlive = true;
        }
    }

    if (key == GLFW_KEY_X) {
        if (action == GLFW_PRESS && params->isDroneAlive) {
            params->isDroneAlive = false;
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

    glfwSetWindowPos(window, 300, 40);
    glfwMakeContextCurrent(window);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, KeyCallback2);
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

    phongShader.setVec3("uDirLight.Position", glm::vec3(0.0, 50.0f, 0.0f));
    phongShader.setVec3("uDirLight.Direction", 0.3, -0.2, 0.6);
    phongShader.setVec3("uDirLight.Ka", glm::vec3(0.15));
    phongShader.setVec3("uDirLight.Kd", glm::vec3(0.18));
    phongShader.setVec3("uDirLight.Ks", glm::vec3(2.0));

    unsigned hudTex = Model::textureFromFile("res/hudTex.png");
    unsigned mapTex = Model::textureFromFile("res/map1.png");
    unsigned mapflipTex = Model::textureFromFile("res/map1Flip.png");
    unsigned mapSpecTex = Model::textureFromFile("res/map1Spec.png");
    unsigned modelSpec = Model::textureFromFile("res/Part2_Metallic.png");
    unsigned sky = Model::textureFromFile("res/stars-night-textured-background.jpg");

    phongShader.setInt("uMaterial.Kd", 0);
    phongShader.setInt("uMaterial.Ks", 1);
    phongShader.setFloat("uMaterial.Shininess", 0.25 * 128);

    glm::mat4 m(1.0f);
    float currentRot = 0;
    float FrameStartTime = 0;
    float FrameEndTime = 0;
    float scannerTime = 0;

    Params params;
    glfwSetWindowUserPointer(window, &params);

    glClearColor(0.13, 0.17, 0.21, 1.0);
    glfwWindowHint(GLFW_SAMPLES, 16);
    glEnable(GL_MULTISAMPLE);

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

        SpawnTargets(&params);
        CheckIfDroneCrashed(&params);
        UpdateTargetsPos(params.dt);

        //Loop
        phongShader.use();
        HandleInput(&params);

        //Camera
        view = glm::lookAt(glm::vec3(0.0, 11, -12.2), glm::vec3(0.0, 2.2, 5), params.cameraUp);
        projectionP = glm::perspective(glm::radians(92.0f), (float)wWidth / (float)wHeight, 0.1f, 300.0f);

        phongShader.setMat4("uView", view);
        phongShader.setMat4("uProjection", projectionP);
        phongShader.setVec3("uViewPos", glm::vec3(0.0, 11, -12.2));

        //SCENE
        //------------------------------------------------------------------------------------------------------------
        
        
        //Mapa velika
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
        m = glm::rotate(m, glm::radians(0.f), glm::vec3(1.0, 0.0, 0.0));
        m = glm::scale(m, glm::vec3(30,1.0,30));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, mapTex, mapSpecTex);

        //Fejk pod ispod mape
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -0.01, 0.0));
        m = glm::rotate(m, glm::radians(0.f), glm::vec3(1.0, 0.0, 0.0));
        m = glm::scale(m, glm::vec3(300, 1.0, 300));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.14, 0.29, 0.22);

        //Centar grada
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 1.5, 5.0));
        m = glm::rotate(m, glm::radians(0.f), glm::vec3(1.0, 0.0, 0.0));
        m = glm::scale(m, glm::vec3(0.5, 2.0, 0.5));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.07, 0.94, 0.92);

        //Nebo(plafon)
        phongShader.setVec3("uDirLight.Ka", glm::vec3(0.3));
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 20.0, 0.0));
        m = glm::scale(m, glm::vec3(60, 1.0, 60));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, sky);
        phongShader.setVec3("uDirLight.Ka", glm::vec3(0.15));

        //PVO base
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 5.5, -13.5));
        m = glm::scale(m, glm::vec3(3, 10.0, 3));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.04, 0.09, 0);

        //Drone
        float coef = 0;
        if (params.isDroneAlive) {
            m = glm::translate(glm::mat4(1.0), params.dronePosition);
            m = glm::scale(m, glm::vec3(0.2));
            phongShader.setMat4("uModel", m);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, modelSpec);
            droneObj.Draw(phongShader);
            coef = 1;
        }

        std::string lightUniform = "uPointLights[";
        lightUniform.append(to_string(NUMBER_OF_HELICOPTERS));
        lightUniform.append("].");
        glm::vec3 pos = params.dronePosition;
        pos.y += 0.3;
        phongShader.setVec3(lightUniform + "Position", pos);
        phongShader.setVec3(lightUniform + "Ka", glm::vec3(0.1f) * coef);
        phongShader.setVec3(lightUniform + "Kd", glm::vec3(0.0, 1.5f, 0) * coef);
        phongShader.setVec3(lightUniform + "Ks", glm::vec3(0.0f, 1.0f, 0) * coef);
        phongShader.setFloat(lightUniform + "Kc", 1.5f);
        phongShader.setFloat(lightUniform + "Kl", 3.0f);
        phongShader.setFloat(lightUniform + "Kq", 2.272f);


        //Svetionik grada
        float time = glfwGetTime() * 15.f;
        float radius = sin(glm::radians(time*2)) + 2;
        glm::vec3 dir = glm::vec3(radius*sin(glm::radians(time)), 4.0, radius*cos(glm::radians(time)));

        phongShader.setVec3("uSpotlights[0].Position", glm::vec3(0,15,5));
        phongShader.setVec3("uSpotlights[0].Direction", -dir);
        phongShader.setVec3("uSpotlights[0].Ka", 0.0, 0.0, 0.0);
        phongShader.setVec3("uSpotlights[0].Kd", glm::vec3(4.f));
        phongShader.setVec3("uSpotlights[0].Ks", glm::vec3(2.0));
        phongShader.setFloat("uSpotlights[0].InnerCutOff", glm::cos(glm::radians(12.0f)));
        phongShader.setFloat("uSpotlights[0].OuterCutOff", glm::cos(glm::radians(18.0f)));
        phongShader.setFloat("uSpotlights[0].Kc", 1.0);
        phongShader.setFloat("uSpotlights[0].Kl", 0.072f);
        phongShader.setFloat("uSpotlights[0].Kq", 0.012f);

        //Render meta
        RenderAliveHelicopters(phongShader, copterObj);

        //Oblaci
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
        float screenRot = 90;
        glm::vec3 mapOffset = glm::vec3(0,-0.4,0.2);
        glm::vec3 posOnMap = glm::vec3(0);

        //Base Map
        if (params.isMapActive) {
            m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, screenHeight, -12.0));
            m = glm::translate(m, mapOffset);
            m = glm::rotate(m, glm::radians(screenRot), glm::vec3(1.0, 0.0, 0.0));
            m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(0.5));
            twoD.setMat4("uModel", m);
            rectangle->Render(&twoD, mapflipTex);

            m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, screenHeight-0.01, -12.0));
            m = glm::translate(m, mapOffset);
            m = glm::rotate(m, glm::radians(screenRot), glm::vec3(1.0, 0.0, 0.0));
            m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(0.6));
            twoD.setMat4("uModel", m);
            rectangle->Render(&twoD, 0.02, 0.13, 0.01);
        }

        //Tacke na mapi koje su mete
        for (int i = 0; i < NUMBER_OF_HELICOPTERS + NUMBER_OF_LOW_HELICOPTERS; i++) {
            if (!targets[i].isAlive) {
                continue;
            }
            float angleScanner = glm::radians(scannerTime);
            glm::vec3 vector = glm::normalize(targets[i].position);
            if (glm::abs(vector.x - sin(angleScanner)) < 0.2 && glm::abs(vector.z - cos(angleScanner)) < 0.2) {
                targets[i].lastKnowsPosition = targets[i].position;
            }

            posOnMap.x = ((targets[i].lastKnowsPosition.x + 15.0f) / 30.0f) * 0.5f - 0.25f;
            posOnMap.z = ((targets[i].lastKnowsPosition.z + 15.0f) / 30.0f) * 0.5f - 0.25f;
            float scale = (targets[i].lastKnowsPosition.y / 15.0f) * 0.015f + 0.004f;
            float offY = 0;
            if (targets[i].isLowFlight) {
                offY = 0.005;
            }
            m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, screenHeight+0.01-offY, -12.0));
            m = glm::translate(m, mapOffset);
            m = glm::translate(m, posOnMap);
            m = glm::rotate(m, glm::radians(screenRot), glm::vec3(1.0, 0.0, 0.0));
            m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(scale));
            twoD.setMat4("uModel", m);
            if (targets[i].isLowFlight) {
                circle->Render(&twoD, 1,0,0);
            }
            else
            {
                circle->Render(&twoD, 1, 1, 0);
            }
           
        }

        //Tacka na mapi koja je dron
        if (params.isDroneAlive) {
            posOnMap.x = ((params.dronePosition.x + 15.0f) / 30.0f) * 0.5f - 0.25f;
            posOnMap.z = ((params.dronePosition.z + 15.0f) / 30.0f) * 0.5f - 0.25f;
            float scale = (params.dronePosition.y / 15.0f) * 0.015f + 0.004f;

            m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, screenHeight + 0.012, -12.0));
            m = glm::translate(m, mapOffset);
            m = glm::translate(m, posOnMap);
            m = glm::rotate(m, glm::radians(screenRot), glm::vec3(1.0, 0.0, 0.0));
            m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(scale));
            twoD.setMat4("uModel", m);
            circle->Render(&twoD, 0, 1, 0);
        }

        //PVO na mapi
        posOnMap.x = ((0 + 15.0f) / 30.0f) * 0.5f - 0.25f;
        posOnMap.z = ((-12 + 15.0f) / 30.0f) * 0.5f - 0.25f;

        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, screenHeight + 0.013, -12.0));
        m = glm::translate(m, mapOffset);
        m = glm::translate(m, posOnMap);
        m = glm::rotate(m, glm::radians(screenRot), glm::vec3(1.0, 0.0, 0.0));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.018));
        twoD.setMat4("uModel", m);
        circle->Render(&twoD, 0, 0, 1);

        //Centar grada na mapi
        posOnMap.x = ((0 + 15.0f) / 30.0f) * 0.5f - 0.25f;
        posOnMap.z = ((5 + 15.0f) / 30.0f) * 0.5f - 0.25f;

        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, screenHeight + 0.007, -12.0));
        m = glm::translate(m, mapOffset);
        m = glm::translate(m, posOnMap);
        m = glm::rotate(m, glm::radians(screenRot), glm::vec3(1.0, 0.0, 0.0));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.02));
        twoD.setMat4("uModel", m);
        rectangle->Render(&twoD, 0, 1, 0.98);


        //Koliko je ostalo dronova
        for (int i = 0; i < params.numberOfDrones; i++) {
            posOnMap.x = -0.25;
            posOnMap.z = -0.25 + 0.015*i;
            m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, screenHeight + 0.009, -12.0));
            m = glm::translate(m, mapOffset);
            m = glm::translate(m, posOnMap);
            m = glm::rotate(m, glm::radians(screenRot), glm::vec3(1.0, 0.0, 0.0));
            m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(0.025,0.01,1));
            twoD.setMat4("uModel", m);
            rectangle->Render(&twoD, 0.81, 0.66, 0.16);
        }

        //Skener
        float scannerScale = 0.255;
        int repetitions = 100;

        twoD.setBool("uT", true);
        for (int i = 0; i < repetitions; i++) {
            m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, screenHeight + 0.0069 - 0.00001 * i, -12.0));
            m = glm::translate(m, mapOffset);
            m = glm::rotate(m, glm::radians(scannerTime - ((45.f / repetitions) * i)), glm::vec3(0.0, 1.0, 0.0));
            m = glm::translate(m, -glm::vec3(0.0, screenHeight + 0.0069 - 0.00001 * i, -12.0));
            m = glm::translate(m, -mapOffset);


            m = glm::translate(m, glm::vec3(0.0, screenHeight + 0.0069 - 0.00001*i, -12.0 + scannerScale / 2));
            m = glm::translate(m, mapOffset);
            m = glm::rotate(m, glm::radians(screenRot), glm::vec3(1.0, 0.0, 0.0));
            m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(0.003, scannerScale, 1));

            double alpha = 1.f + (0.001f - 1.f) * ((double)i / repetitions);
            twoD.setFloat("uA", alpha);
            twoD.setMat4("uModel", m);
            rectangle->Render(&twoD, 0.05, 0.51, 0.01);
        }
        twoD.setBool("uT", false);

        scannerTime += params.dt * 80;

        //3D Ostatak
        //LED Da je dron lansiran
        phongShader.use();
        posOnMap.x = 0.26;
        posOnMap.z = -0.26;
        float ledLightCoef = 0;

        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, screenHeight + 0.01, -12.0));
        m = glm::translate(m, mapOffset);
        m = glm::translate(m, posOnMap);
        m = glm::rotate(m, glm::radians(screenRot), glm::vec3(1.0, 0.0, 0.0));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.02));
        phongShader.setMat4("uModel", m);
        if (params.isDroneAlive) {
            simpleCube->Render(&phongShader, 0, 1, 0);
            ledLightCoef = 1;
        }
        else
        {
            simpleCube->Render(&phongShader, 1, 1, 1);
        }

        glm::vec3 ledPos = glm::vec3(m[3]);

        lightUniform = "uPointLights[";
        lightUniform.append(to_string(NUMBER_OF_HELICOPTERS+1));
        lightUniform.append("].");
        pos = ledPos;
        pos.y += 0.15;
  
        phongShader.setVec3(lightUniform + "Position", pos);
        phongShader.setVec3(lightUniform + "Ka", glm::vec3(0,0.2f,0)* ledLightCoef);
        phongShader.setVec3(lightUniform + "Kd", glm::vec3(0, 5, 0)* ledLightCoef);
        phongShader.setVec3(lightUniform + "Ks", glm::vec3(0, 2.0, 0)* ledLightCoef);
        phongShader.setFloat(lightUniform + "Kc", 5.5f);
        phongShader.setFloat(lightUniform + "Kl", 5.0f);
        phongShader.setFloat(lightUniform + "Kq", 8.272f);

        //Staklo ekrana
        phongShader.use();
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, screenHeight + 0.01, -12.0));
        m = glm::translate(m, mapOffset);
        m = glm::rotate(m, glm::radians(screenRot-90), glm::vec3(1.0, 0.0, 0.0));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.55, 0.05, 0.55));
        phongShader.setMat4("uModel", m);
        phongShader.setBool("uTransp", true);
        if (params.isMapActive) {
            phongShader.setFloat("uAlpha", 0.3);
        }
        else
        {
            phongShader.setFloat("uAlpha", 0.05);
        }
        simpleCube->Render(&phongShader, 0.04, 0.39, 0.03);
        phongShader.setBool("uTransp", false);


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


