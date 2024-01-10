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

unsigned hudTex;
unsigned kockaDif;
unsigned kockaSpec;
unsigned asphaltD;
unsigned asphaltS;

GameObject* simpleCube;
GameObject* simpleCube2;
Model lija;

bool firstMouse = true;
double lastX;
double lastY;

struct Params {
    float dt;
    bool isFps = true;
    bool freeCam = true;

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

    glm::vec3 forward = glm::vec3(0, 0, 1);
    glm::vec3 offset = glm::vec3(0);
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

static void DrawScene(Shader& shader, Params &params, bool isBack = false) {
    glm::mat4 m = glm::mat4(1.0f);

    //test
    m = glm::translate(glm::mat4(1.0), glm::vec3(-10.0, 0.0, 0.0));
    m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
    m = glm::scale(m, glm::vec3(1.0, 1.0, 1.0));
    shader.setMat4("uModel", m);
    simpleCube->Render(&shader, 0, 1, 1);

    //Road
    m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -1.0, 0.0));
    m = glm::scale(m, glm::vec3(500,1.0,500));
    shader.setMat4("uModel", m);
    simpleCube2->Render(&shader, asphaltD, kockaSpec);

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
    if (params.headlights) {
        if (params.longLights) {
            lightIntA = zeroVec;
            lightIntD = glm::vec3(12);
            lightIntS = glm::vec3(30.0);
        }
        else {
            lightIntA = zeroVec;
            lightIntD = glm::vec3(5);
            lightIntS = glm::vec3(30.0);
        }
    }

    shader.setVec3("uSpotlights[0].Position", light1Pos);
    shader.setVec3("uSpotlights[0].Direction", params.forward);
    shader.setVec3("uSpotlights[0].Ka", lightIntA);
    shader.setVec3("uSpotlights[0].Kd", lightIntD);
    shader.setVec3("uSpotlights[0].Ks", lightIntS);
    shader.setFloat("uSpotlights[0].InnerCutOff", glm::cos(glm::radians(25.0f)));
    shader.setFloat("uSpotlights[0].OuterCutOff", glm::cos(glm::radians(35.0f)));
    shader.setFloat("uSpotlights[0].Kc", 0.2);
    shader.setFloat("uSpotlights[0].Kl", 0.092f);
    shader.setFloat("uSpotlights[0].Kq", 0.0032f);

    shader.setVec3("uSpotlights[1].Position", light2Pos);
    shader.setVec3("uSpotlights[1].Direction", params.forward);
    shader.setVec3("uSpotlights[1].Ka", lightIntA);
    shader.setVec3("uSpotlights[1].Kd", lightIntD);
    shader.setVec3("uSpotlights[1].Ks", lightIntS);
    shader.setFloat("uSpotlights[1].InnerCutOff", glm::cos(glm::radians(25.0f)));
    shader.setFloat("uSpotlights[1].OuterCutOff", glm::cos(glm::radians(35.0f)));
    shader.setFloat("uSpotlights[1].Kc", 0.2);
    shader.setFloat("uSpotlights[1].Kl", 0.092f);
    shader.setFloat("uSpotlights[1].Kq", 0.0032f);


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


    //CAR
    //Handle gears here
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

    if (params->gasDown) {
        int reverse = 1;
        if (params->reverse) {
            reverse = -1;
        }

        params->velocity += 5 * params->dt * reverse * (6 - params->gear)/2;
    }
    else {
        float damp = 2;
        if (params->breakeDown) {
            damp = 10;
        }

        if (glm::abs(params->velocity) < 0.05) {
            params->velocity = 0;
            params->gear = 0;
        }


        if (params->velocity > 0) {
            params->velocity -= 6 * damp * params->dt;
        }
        else if (params->velocity < 0) {
            params->velocity += 6 * damp * params->dt;
        }

    }

    if (glm::abs(params->velocity) > 0) {
        if (params->turnLeft) {
            params->rotation += 100 * params->dt;
        }
        else if (params->turnRight) {
            params->rotation -= 100 * params->dt;
        }
    }
    params->velocity = glm::clamp(params->velocity, -20.f, 100.f);

    params->offset += params->forward * params->velocity * params->dt;

    //cout << params->velocity << " g: " << params->gear << " R: " << params->rotation << endl;

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

    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        params->freeCam = !params->freeCam;
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
    simpleCube = new GameObject(cubeVertices);

    float tiling = 100.f;
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

    lija = Model("res/low-poly-fox.obj");

    Shader phongShader("phong.vert", "phong.frag");
    Shader hudShader("hud.vert", "hud.frag");
    Shader twoD("twoD.vert", "twoD.frag");

    phongShader.use();

    glm::mat4 view;
    glm::mat4 projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
    phongShader.setMat4("uProjection", projectionP);

    phongShader.setVec3("uDirLight.Position", 0.0, 10, 0.0);
    phongShader.setVec3("uDirLight.Direction", 1, -1, 0);
    phongShader.setVec3("uDirLight.Ka", glm::vec3(0.3));
    phongShader.setVec3("uDirLight.Kd", glm::vec3(0.4));
    phongShader.setVec3("uDirLight.Ks", glm::vec3(1));

    phongShader.setVec3("uPointLights[0].Position", glm::vec3(-1000));
    phongShader.setVec3("uPointLights[0].Ka", glm::vec3(0.2));
    phongShader.setVec3("uPointLights[0].Kd", glm::vec3(0.2));
    phongShader.setVec3("uPointLights[0].Ks", glm::vec3(1.0f));
    phongShader.setFloat("uPointLights[0].Kc", 1.5f);
    phongShader.setFloat("uPointLights[0].Kl", 1.0f);
    phongShader.setFloat("uPointLights[0].Kq", 0.272f);

    hudTex = Model::textureFromFile("res/hudTex.png");
    kockaDif = Model::textureFromFile("res/container_diffuse.png");
    kockaSpec = Model::textureFromFile("res/container_specular.png");
    asphaltD = Model::textureFromFile("res/aspd.png");
    asphaltS = Model::textureFromFile("res/aspc.png");

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

        if (params.freeCam) {
            view = glm::lookAt(params.position,params.position + params.cameraFront,params.cameraUp);
            phongShader.setVec3("uViewPos", params.position);
        }
        else {
            view = glm::lookAt(camPos, camPos + params.forward, glm::vec3(0, 1, 0));
            phongShader.setVec3("uViewPos", camPos);
        }
        phongShader.setMat4("uView", view);

        DrawScene(phongShader, params);


        //glScissor(0, 0, 500, 500);
        
        glScissor(0, 0, wWidth / 5, wHeight / 5);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, wWidth/5, wHeight/5);
        camPos = glm::vec3(0, 3, 0) + params.offset;
        view = glm::lookAt(camPos, camPos - params.forward, glm::vec3(0, 1, 0));
        //view = glm::lookAt(params.position, params.position + params.cameraFront, params.cameraUp);
        phongShader.setMat4("uView", view);
        phongShader.setVec3("uViewPos", camPos);
        DrawScene(phongShader, params);

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


