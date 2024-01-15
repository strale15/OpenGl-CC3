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
#include <ctime> // for time()
#include <cstdlib> // for rand() and srand()

#define TREES 500

const unsigned int wWidth = 1920;
const unsigned int wHeight = 1080;

unsigned hudTex;
unsigned kockaDif;
unsigned kockaSpec;
unsigned asphaltD;
unsigned asphaltS;
unsigned speedTex;

GameObject* simpleCube;
GameObject* simpleCube2;
GameObject* rectangle;
Model astronautModel;

bool firstMouse = true;
double lastX;
double lastY;

float signalCounter = 0;
bool signalOn = false;

int randomCoords[TREES*2];

struct Params {
    float dt = 0;

    glm::vec3 cameraFront = glm::vec3(0.0, 0.0, 1.0);
    glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

    glm::vec3 position = glm::vec3(0.0, 0.0, -1.0);
    glm::vec3 objPos = glm::vec3(0.0, 0.0, 0.0);

    bool gasDown = false;
    bool breakeDown = false;
    float velocity = 0;
    int gear = 0;
    bool reverse = false;

    bool headlights = false;
    bool longLights = false;

    bool turnLeft = false;
    bool turnRight = false;
    float rotation = 0;

    float fuel = 100;

    bool leftSignal = false;
    bool rightSignal = false;

    glm::vec3 forward = glm::vec3(0, 0, 1);
    glm::vec3 offset = glm::vec3(0);
};

static void RandomCoords() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (int i = 0; i < TREES * 2; i++) {

        gas:
        int randomNumber = std::rand() % (2000 + 1) - 1000;
        if (glm::abs(randomNumber) < 30) {
            goto gas;
        }
        randomCoords[i] = randomNumber;
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

static void DrawScene(Shader& shader, Params &params) {
    glm::mat4 m = glm::mat4(1.0f);

    //Road
    m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -1.0, 0.0));
    m = glm::scale(m, glm::vec3(2000,1.0,2000));
    shader.setMat4("uModel", m);
    simpleCube2->Render(&shader, asphaltD, asphaltS);

    //Car
    //Base
    m = glm::translate(glm::mat4(1.0), params.offset);
    m = glm::rotate(m, glm::radians(params.rotation), glm::vec3(0.0, 1.0, 0.0));
    m = glm::translate(m, -params.offset);
    m = glm::translate(m, params.offset);

    m = glm::translate(m, glm::vec3(0.0, 0.0, 0.0));
    m = glm::scale(m, glm::vec3(6.0, 1.0, 10.0));
    shader.setMat4("uModel", m);
    simpleCube->Render(&shader, 0, 1, 1);

    params.forward = glm::normalize(m[2]);
    glm::vec3 carFrontPos = glm::vec3(m[3]);

    //Side
    m = glm::translate(glm::mat4(1.0), params.offset);
    m = glm::rotate(m, glm::radians(params.rotation), glm::vec3(0.0, 1.0, 0.0));
    m = glm::translate(m, -params.offset);
    m = glm::translate(m, params.offset);

    m = glm::translate(m, glm::vec3(3.0, 0.0, 0.0));
    m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 0.0, 1.0));
    m = glm::scale(m, glm::vec3(3.0, 1.0, 10.0));
    shader.setMat4("uModel", m);
    simpleCube->Render(&shader, 0, 1, 1);

    m = glm::translate(glm::mat4(1.0), params.offset);
    m = glm::rotate(m, glm::radians(params.rotation), glm::vec3(0.0, 1.0, 0.0));
    m = glm::translate(m, -params.offset);
    m = glm::translate(m, params.offset);

    m = glm::translate(m, glm::vec3(-3.0, 0.0, 0.0));
    m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 0.0, 1.0));
    m = glm::scale(m, glm::vec3(3.0, 1.0, 10.0));
    shader.setMat4("uModel", m);
    simpleCube->Render(&shader, 0, 1, 1);

    m = glm::translate(glm::mat4(1.0), params.offset);
    m = glm::rotate(m, glm::radians(params.rotation), glm::vec3(0.0, 1.0, 0.0));
    m = glm::translate(m, -params.offset);
    m = glm::translate(m, params.offset);

    m = glm::translate(m, glm::vec3(0.0, 0.0, -4.5));
    m = glm::scale(m, glm::vec3(6.0, 3.0, 1.0));
    shader.setMat4("uModel", m);
    simpleCube->Render(&shader, 0, 1, 1);

    //Inst
    m = glm::translate(glm::mat4(1.0), params.offset);
    m = glm::rotate(m, glm::radians(params.rotation), glm::vec3(0.0, 1.0, 0.0));
    m = glm::translate(m, -params.offset);
    m = glm::translate(m, params.offset);

    m = glm::translate(m, glm::vec3(0.0, 0.0, 4.0));
    m = glm::scale(m, glm::vec3(5.0, 3.0, 2.0));
    shader.setMat4("uModel", m);
    simpleCube->Render(&shader, 1, 1, 1);

    //Bobblehead
    m = glm::translate(glm::mat4(1.0), params.offset);
    m = glm::rotate(m, glm::radians(params.rotation), glm::vec3(0.0, 1.0, 0.0));
    m = glm::translate(m, -params.offset);
    m = glm::translate(m, params.offset);

    m = glm::translate(m, glm::vec3(-1.7, 1.5, 3.5));
    m = glm::rotate(m, glm::radians(160.f), glm::vec3(0.0, 1.0, 0.0));
    m = glm::scale(m, glm::vec3(0.2));
    shader.setMat4("uModel", m);
    astronautModel.Draw(shader);

    //Trees
    for (int i = 0; i < TREES; i++) {
        m = glm::translate(glm::mat4(1.0), glm::vec3(randomCoords[i], 9.0, randomCoords[i + TREES]));
        m = glm::scale(m, glm::vec3(1.0, 20.0, 1.0));
        shader.setMat4("uModel", m);
        simpleCube->Render(&shader, 0, 1, 0);
    }

    //Glass
    m = glm::translate(glm::mat4(1.0), params.offset);
    m = glm::rotate(m, glm::radians(params.rotation), glm::vec3(0.0, 1.0, 0.0));
    m = glm::translate(m, -params.offset);
    m = glm::translate(m, params.offset);

    m = glm::translate(m, glm::vec3(0.0, 3.0, 3.7));
    m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
    m = glm::rotate(m, glm::radians(30.f), glm::vec3(1.0, 0.0, 0.0));
    m = glm::scale(m, glm::vec3(6.0, 5.0, 0.1));
    shader.setMat4("uModel", m);
    shader.setBool("uTransp", true);
    simpleCube->Render(&shader, 0, 0, 1);
    shader.setBool("uTransp", false);

    //Lights
    glm::vec3 zeroVec = glm::vec3(0);
    glm::vec3 right = glm::cross(params.forward, glm::vec3(0, 1, 0));
    right = glm::normalize(right);
    glm::vec3 light1Pos = carFrontPos + params.forward * 5.f + right * 3.f;
    glm::vec3 light2Pos = carFrontPos + params.forward * 5.f - right * 3.f;

    glm::vec3 lightIntA = zeroVec;
    glm::vec3 lightIntD = zeroVec;
    glm::vec3 lightIntS = zeroVec;
    float kc = 0.1;
    float kl = 0.052;
    float kq = 0.00032;
    float inner = 20;
    float outer = 30;
    if (params.headlights) {
        if (params.longLights) {
            lightIntA = zeroVec;
            lightIntD = glm::vec3(12);
            lightIntS = glm::vec3(60.0);

            float kc = 0.02;
            float kl = 0.0092;
            float kq = 0.000032;
            inner = 30;
            outer = 45;
        }
        else {
            lightIntA = zeroVec;
            lightIntD = glm::vec3(5);
            lightIntS = glm::vec3(60.0);
        }
    }

    shader.setVec3("uSpotlights[0].Position", light1Pos);
    shader.setVec3("uSpotlights[0].Direction", params.forward);
    shader.setVec3("uSpotlights[0].Ka", lightIntA);
    shader.setVec3("uSpotlights[0].Kd", lightIntD);
    shader.setVec3("uSpotlights[0].Ks", lightIntS);
    shader.setFloat("uSpotlights[0].InnerCutOff", glm::cos(glm::radians(inner)));
    shader.setFloat("uSpotlights[0].OuterCutOff", glm::cos(glm::radians(outer)));
    shader.setFloat("uSpotlights[0].Kc", kc);
    shader.setFloat("uSpotlights[0].Kl", kl);
    shader.setFloat("uSpotlights[0].Kq", kq);


    shader.setVec3("uSpotlights[1].Position", light2Pos);
    shader.setVec3("uSpotlights[1].Direction", params.forward);
    shader.setVec3("uSpotlights[1].Ka", lightIntA);
    shader.setVec3("uSpotlights[1].Kd", lightIntD);
    shader.setVec3("uSpotlights[1].Ks", lightIntS);
    shader.setFloat("uSpotlights[1].InnerCutOff", glm::cos(glm::radians(inner)));
    shader.setFloat("uSpotlights[1].OuterCutOff", glm::cos(glm::radians(outer)));
    shader.setFloat("uSpotlights[1].Kc", kc);
    shader.setFloat("uSpotlights[1].Kl", kl);
    shader.setFloat("uSpotlights[1].Kq", kq);
}

static void HandleInput(Params* params) {
    //CAR
    if (params->velocity > 80) {
        params->gear = 5;
    }
    else if (params->velocity > 60 && params->velocity <= 80) {
        params->gear = 4;
    }
    else if (params->velocity > 40 && params->velocity <= 60) {
        params->gear = 3;
    }
    else if (params->velocity > 20 && params->velocity <= 40) {
        params->gear = 2;
    }
    else if (params->velocity >= 0 && params->velocity <= 20) {
        params->gear = 1;
    }

    if (params->gasDown && !params->breakeDown) {
        int reverse = 1;
        if (params->reverse) {
            reverse = -3;
        }

        params->velocity += 5 * params->dt * reverse * (6 - params->gear)/2;
    }
    else {
        float damp = 2;
        if (params->breakeDown) {
            damp = 5;
        }

        if (glm::abs(params->velocity) < 0.05) {
            params->velocity = 0;
            params->gear = 0;
        }


        if (params->velocity > 0) {
            params->velocity -= 7 * damp * params->dt;
        }
        else if (params->velocity < 0) {
            params->velocity += 7 * damp * params->dt;
        }

    }

    if (glm::abs(params->velocity) > 0) {
        if (params->turnLeft) {
            params->rotation += 50 * params->dt;
        }
        else if (params->turnRight) {
            params->rotation -= 50 * params->dt;
        }
    }
    params->velocity = glm::clamp(params->velocity, -20.f, 100.f);

    params->offset += params->forward * params->velocity * params->dt;

    if (params->velocity > 0) {
        params->fuel -= 3 * params->dt;
    }
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    Params* params = (Params*)glfwGetWindowUserPointer(window);

    //CAR
    if (key == GLFW_KEY_UP) {
        if (action == GLFW_PRESS) {
            params->gasDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->gasDown = false;
        }
    }
    if (key == GLFW_KEY_DOWN) {
        if (action == GLFW_PRESS) {
            params->breakeDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->breakeDown = false;
        }
    }
    if (key == GLFW_KEY_LEFT) {
        if (action == GLFW_PRESS) {
            params->turnLeft = true;
        }
        else if (action == GLFW_RELEASE) {
            params->turnLeft = false;
        }
    }
    if (key == GLFW_KEY_RIGHT) {
        if (action == GLFW_PRESS) {
            params->turnRight = true;
        }
        else if (action == GLFW_RELEASE) {
            params->turnRight = false;
        }
    }
    if (key == GLFW_KEY_F) {
        if (action == GLFW_PRESS) {
            params->headlights = !params->headlights;
        }
    }
    if (key == GLFW_KEY_G) {
        if (action == GLFW_PRESS) {
            params->longLights = !params->longLights;
        }
    }
    if (key == GLFW_KEY_H) {
        if (action == GLFW_PRESS) {
            params->reverse = !params->reverse;
        }
    }
    if (key == GLFW_KEY_Q) {
        if (action == GLFW_PRESS) {
            params->leftSignal = !params->leftSignal;
            if (params->rightSignal) {
                params->rightSignal = false;
            }
            signalOn = true;
            signalCounter = 0;
        }
    }
    if (key == GLFW_KEY_E) {
        if (action == GLFW_PRESS) {
            params->rightSignal = !params->rightSignal;
            if (params->leftSignal) {
                params->leftSignal = false;
            }
            signalOn = true;
            signalCounter = 0;
        }
    }
}

int main()
{
    RandomCoords();
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
    simpleCube = new GameObject(cubeVertices);

    float tiling = 400.f;
    std::vector<float> cubeVertices2 = {
        // X     Y     Z     NX    NY    NZ    U     V    FRONT SIDE
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, tiling, // L D
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, tiling, tiling, // R D
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // L U
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, tiling, tiling, // R D
        0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, tiling, 0.0f, // R U
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // L U
        // LEFT SIDE
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, tiling, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, tiling, // L U
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, tiling, 0.0f, // R D
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, tiling, tiling, // R U
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, tiling, // L U
        // RIGHT SIDE
        0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, tiling, 0.0f, // R D
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, tiling, // L U
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, tiling, 0.0f, // R D
        0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, tiling, tiling, // R U
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, tiling, // L U
        // BOTTOM SIDE
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, tiling, 0.0f, // R D
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, tiling, // L U
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, tiling, 0.0f, // R D
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, tiling, tiling, // R U
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, tiling, // L U
        // TOP SIDE
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, tiling, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, tiling, // L U
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, tiling, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, tiling, tiling, // R U
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, tiling, // L U
        // BACK SIDE
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, tiling, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, tiling, // L U
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,tiling, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, tiling, tiling, // R U
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f,tiling, // L U
    };
    simpleCube2 = new GameObject(cubeVertices2);

    std::vector<float> vertices = {
        // Positions      // UVs
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,  // Vertex 1
        0.5f, -0.5f, 0.0f, 1.0f, 1.0f,  // Vertex 2
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f,  // Vertex 3

        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,  // Vertex 1 (Repeated)
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f,  // Vertex 3 (Repeated)
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f   // Vertex 4
    };
    rectangle = new GameObject(vertices, true);

    astronautModel = Model("res/Astronaut.obj");

    Shader phongShader("phong.vert", "phong.frag");
    Shader hudShader("hud.vert", "hud.frag");
    Shader twoD("twoD.vert", "twoD.frag");

    phongShader.use();

    glm::mat4 view;
    glm::mat4 projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 500.0f);
    phongShader.setMat4("uProjection", projectionP);

    phongShader.setVec3("uDirLight.Position", 0.0, 10, 0.0);
    phongShader.setVec3("uDirLight.Direction", 0.3, -1, 0);
    phongShader.setVec3("uDirLight.Ka", glm::vec3(1, 0.43, 0.38)/ 6.f);
    phongShader.setVec3("uDirLight.Kd", glm::vec3(1, 0.43, 0.38)/ 4.f);
    phongShader.setVec3("uDirLight.Ks", glm::vec3(1));

    hudTex = Model::textureFromFile("res/hudTex.png");
    asphaltD = Model::textureFromFile("res/aspd.png");
    asphaltS = Model::textureFromFile("res/aspc.png");
    speedTex = Model::textureFromFile("res/speed.png");

    phongShader.setInt("uMaterial.Kd", 0);
    phongShader.setInt("uMaterial.Ks", 1);
    phongShader.setFloat("uMaterial.Shininess", 0.25 * 128);

    glm::mat4 m(1.0f);
    float currentRot = 0;
    float FrameStartTime = 0;
    float FrameEndTime = 0;

    Params params;
    glfwSetWindowUserPointer(window, &params);

    glClearColor(1, 0.43, 0.38, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    while (!glfwWindowShouldClose(window))
    {
        FrameStartTime = glfwGetTime();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        //Loop
        phongShader.use();
        HandleInput(&params);

        //Camera
        glScissor(0, 0, wWidth, wHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, wWidth, wHeight);

        glm::vec3 camPos = glm::vec3(0, 3, 0) + params.offset;
        view = glm::lookAt(camPos, camPos + params.forward, glm::vec3(0, 1, 0));
        phongShader.setVec3("uViewPos", camPos);
        phongShader.setMat4("uView", view);

        //Instrument
        twoD.use();
        twoD.setMat4("uView", view);
        twoD.setMat4("uProjection", projectionP);

        //Spped Indicator
        float rotation = (180 * glm::abs(params.velocity) / 100.0);

        m = glm::translate(glm::mat4(1.0), params.offset);
        m = glm::rotate(m, glm::radians(params.rotation), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.offset);

        m = glm::translate(m, glm::vec3(1.5, 0.55, 2.97) + params.offset);
        m = glm::rotate(m, glm::radians(-90 + rotation), glm::vec3(0.0, 0.0, 1.0));
        m = glm::translate(m, -glm::vec3(1.5, 0.55, 2.97) - params.offset);

        m = glm::translate(m, params.offset);

        m = glm::translate(m, glm::vec3(1.5, 0.85, 2.97));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.05, 0.6, 1.0));
        twoD.setMat4("uModel", m);
        if (rotation > 55) {
            rectangle->Render(&twoD, 1, 0, 0);
        }
        else {
            rectangle->Render(&twoD, 0,0,1);
        }

        //Fuel
        float scale = 0.85 * params.fuel / 100.0;
        scale += 0.05;
        float yoffset = (0.9 - scale) / 2;

        m = glm::translate(glm::mat4(1.0), params.offset);
        m = glm::rotate(m, glm::radians(params.rotation), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.offset);
        m = glm::translate(m, params.offset);

        m = glm::translate(m, glm::vec3(-2.1, 1.0 - yoffset, 2.98));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.3, scale, 1.0));
        twoD.setMat4("uModel", m);
        rectangle->Render(&twoD, 0,1,0);

        //Gear
        m = glm::translate(glm::mat4(1.0), params.offset);
        m = glm::rotate(m, glm::radians(params.rotation), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.offset);
        m = glm::translate(m, params.offset);

        m = glm::translate(m, glm::vec3(-1.4, 1.0, 2.98));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.3, 0.3, 1.0));
        twoD.setMat4("uModel", m);

        glm::vec3 gearPos = glm::vec3(m[3]);
        gearPos -= params.forward * 0.3f;

        phongShader.use();

        phongShader.setVec3("uPointLights[2].Position", gearPos);
        phongShader.setFloat("uPointLights[2].Kc", 0.5f);
        phongShader.setFloat("uPointLights[2].Kl", 0.6f);
        phongShader.setFloat("uPointLights[2].Kq", 10.0f);

        if (params.gear == 0) {
            glm::vec3 zeroVec = glm::vec3(0.0f);
            phongShader.setVec3("uPointLights[2].Ka", zeroVec);
            phongShader.setVec3("uPointLights[2].Kd", zeroVec);
            phongShader.setVec3("uPointLights[2].Ks", zeroVec);

            twoD.use();
            rectangle->Render(&twoD, 0.9, 0.9, 0.9);
        }
        else if(params.gear == 1)
        {
            phongShader.setVec3("uPointLights[2].Ka", glm::vec3(0, 0, 0));
            phongShader.setVec3("uPointLights[2].Kd", glm::vec3(0.7, 0, 0));
            phongShader.setVec3("uPointLights[2].Ks", glm::vec3(1, 0, 0));

            twoD.use();
            rectangle->Render(&twoD, 1, 0, 0);
        }
        else if (params.gear == 2)
        {
            phongShader.setVec3("uPointLights[2].Ka", glm::vec3(0, 0, 0));
            phongShader.setVec3("uPointLights[2].Kd", glm::vec3(0.98, 0.65, 0)*1.5f);
            phongShader.setVec3("uPointLights[2].Ks", glm::vec3(0.98, 0.65, 0));

            twoD.use();
            rectangle->Render(&twoD, 0.98, 0.65, 0);
        }
        else if (params.gear == 3)
        {
            phongShader.setVec3("uPointLights[2].Ka", glm::vec3(0, 0, 0));
            phongShader.setVec3("uPointLights[2].Kd", glm::vec3(0.52, 0, 1) * 1.5f);
            phongShader.setVec3("uPointLights[2].Ks", glm::vec3(0.52, 0, 1));

            twoD.use();
            rectangle->Render(&twoD, 0.52, 0, 1);
        }
        else if (params.gear == 4)
        {
            phongShader.setVec3("uPointLights[2].Ka", glm::vec3(0, 0, 0));
            phongShader.setVec3("uPointLights[2].Kd", glm::vec3(0, 0, 1) * 1.9f);
            phongShader.setVec3("uPointLights[2].Ks", glm::vec3(0, 0, 1));

            twoD.use();
            rectangle->Render(&twoD, 0, 0, 1);
        }
        else
        {
            phongShader.setVec3("uPointLights[2].Ka", glm::vec3(0, 0, 0));
            phongShader.setVec3("uPointLights[2].Kd", glm::vec3(0, 1, 0));
            phongShader.setVec3("uPointLights[2].Ks", glm::vec3(0, 1, 0));

            twoD.use();
            rectangle->Render(&twoD, 0, 1, 0);
        }

        //Turn signal
        signalCounter += params.dt/2;
        if (signalCounter > 0.2) {
            signalCounter = 0;
            signalOn = !signalOn;
        }

        m = glm::translate(glm::mat4(1.0), params.offset);
        m = glm::rotate(m, glm::radians(params.rotation), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.offset);
        m = glm::translate(m, params.offset);

        m = glm::translate(m, glm::vec3(-0.5, 1.0, 2.98));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.3, 0.3, 1.0));
        twoD.setMat4("uModel", m);
        rectangle->Render(&twoD, 1, 1, 0);

        glm::vec3 turn1Pos = glm::vec3(m[3]);
        turn1Pos -= params.forward * 0.3f;

        phongShader.use();
        if (!signalOn || !params.rightSignal) {
            glm::vec3 zeroVec = glm::vec3(0.0f);
            phongShader.setVec3("uPointLights[0].Ka", zeroVec);
            phongShader.setVec3("uPointLights[0].Kd", zeroVec);
            phongShader.setVec3("uPointLights[0].Ks", zeroVec);
        }
        else
        {
            phongShader.setVec3("uPointLights[0].Ka", glm::vec3(0, 0, 0));
            phongShader.setVec3("uPointLights[0].Kd", glm::vec3(2, 2, 0));
            phongShader.setVec3("uPointLights[0].Ks", glm::vec3(1, 1, 0));
        }
        phongShader.setVec3("uPointLights[0].Position", turn1Pos);
        phongShader.setFloat("uPointLights[0].Kc", 1.0f);
        phongShader.setFloat("uPointLights[0].Kl", 0.6f);
        phongShader.setFloat("uPointLights[0].Kq", 10.0f);
        twoD.use();

        //Turn signal 2
        m = glm::translate(glm::mat4(1.0), params.offset);
        m = glm::rotate(m, glm::radians(params.rotation), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.offset);
        m = glm::translate(m, params.offset);

        m = glm::translate(m, glm::vec3(0.3, 1.0, 2.98));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.3, 0.3, 1.0));
        twoD.setMat4("uModel", m);
        rectangle->Render(&twoD, 1, 1, 0);

        glm::vec3 turn2Pos = glm::vec3(m[3]);
        turn2Pos -= params.forward * 0.3f;

        phongShader.use();
        if (!signalOn || !params.leftSignal) {
            glm::vec3 zeroVec = glm::vec3(0.0f);
            phongShader.setVec3("uPointLights[1].Ka", zeroVec);
            phongShader.setVec3("uPointLights[1].Kd", zeroVec);
            phongShader.setVec3("uPointLights[1].Ks", zeroVec);
        }
        else
        {
            phongShader.setVec3("uPointLights[1].Ka", glm::vec3(0, 0, 0));
            phongShader.setVec3("uPointLights[1].Kd", glm::vec3(2, 2, 0));
            phongShader.setVec3("uPointLights[1].Ks", glm::vec3(1, 1, 0));
        }
        phongShader.setVec3("uPointLights[1].Position", turn2Pos);
        phongShader.setFloat("uPointLights[1].Kc", 1.0f);
        phongShader.setFloat("uPointLights[1].Kl", 0.6f);
        phongShader.setFloat("uPointLights[1].Kq", 10.0f);

        phongShader.use();
        DrawScene(phongShader, params);
        
        //SpeedMeter
        twoD.use();
        m = glm::translate(glm::mat4(1.0), params.offset);
        m = glm::rotate(m, glm::radians(params.rotation), glm::vec3(0.0, 1.0, 0.0));
        m = glm::translate(m, -params.offset);
        m = glm::translate(m, params.offset);

        m = glm::translate(m, glm::vec3(1.5, 1.0, 2.98));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(1.0 * 1.9, 1.0, 1.0));
        twoD.setMat4("uModel", m);
        rectangle->Render(&twoD, speedTex);

        //HUD
        DrawHud(hudShader, hudTex);

        phongShader.use();
        //Mirror
        glScissor(0, 0, wWidth / 4, wHeight / 4);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, wWidth/ 4, wHeight/ 4);
        camPos = glm::vec3(0, 3, 0) + params.offset;
        view = glm::lookAt(camPos, camPos - params.forward, glm::vec3(0, 1, 0));
        phongShader.setMat4("uView", view);
        phongShader.setVec3("uViewPos", camPos);
        DrawScene(phongShader, params);

        glfwSwapBuffers(window);
        glfwPollEvents();

        FrameEndTime = glfwGetTime();
        params.dt = FrameEndTime - FrameStartTime;
    }

    glfwTerminate();
    return 0;
}


