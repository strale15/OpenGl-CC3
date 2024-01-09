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
    bool test1 = true;

    bool isCurosIn;
    double xPosC = 0.0;
    double yPosC = 0.0;

    double frontX;
    double frontY;
    double frontZ;
    glm::vec3 cameraFront = glm::vec3(0.0, -0.5, 0.866025);
    glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

    glm::vec3 position = glm::vec3(0.0, 3.0, 0.0);
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

    GLFWwindow* window = glfwCreateWindow(wWidth, wHeight, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Window fail!\n" << std::endl;
        glfwTerminate();
        return -2;
    }
    glfwMakeContextCurrent(window);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursosPosCallback);

    if (glewInit() !=GLEW_OK)
    {
        std::cout << "GLEW fail! :(\n" << std::endl;
        return -3;
    }

    //START
    std::vector<float> cubeVertices = {
        // X     Y     Z     NX    NY    NZ    U     V    FRONT SIDE
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // L U
         0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // R U
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // L U
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


    Model lija("res/low-poly-fox.obj");

    Shader phongShader("phong.vert", "phong.frag");
    Shader hudShader("hud.vert", "hud.frag");


    phongShader.use();
    glm::mat4 model = glm::mat4(1.0f);
    phongShader.setMat4("uModel", model);

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    phongShader.setMat4("uView", view);

    glm::mat4 projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
    phongShader.setMat4("uProjection", projectionP);

    phongShader.setVec3("uViewPos", 0.0, 0.0, 5.0);

    phongShader.setVec3("uDirLight.Position", 0.0, 5, 0.0);
    phongShader.setVec3("uDirLight.Direction", 0.1, -5, 0.1);
    phongShader.setVec3("uDirLight.Ka", glm::vec3(255.0 / 255 / 10, 238.0 / 255 / 10, 204.0 / 255 / 10));
    phongShader.setVec3("uDirLight.Kd", glm::vec3(255.0 / 255 / 10, 238.0 / 255 / 10, 204.0 / 255 / 10));
    phongShader.setVec3("uDirLight.Ks", glm::vec3(1.0, 1.0, 1.0));

    phongShader.setVec3("uSpotlights[0].Position", 0.0, 10.0, 0.0);
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

    phongShader.setVec3("uPointLights[0].Position", glm::vec3(0.0, -2.0, 0.0));
    phongShader.setVec3("uPointLights[0].Ka", glm::vec3(230.0 / 255 / 0.1, 92.0 / 255 / 0.1, 0.0f));
    phongShader.setVec3("uPointLights[0].Kd", glm::vec3(230.0 / 255 / 50, 92.0 / 255 / 50, 0.0f));
    phongShader.setVec3("uPointLights[0].Ks", glm::vec3(1.0f));
    phongShader.setFloat("uPointLights[0].Kc", 1.5f);
    phongShader.setFloat("uPointLights[0].Kl", 1.0f);
    phongShader.setFloat("uPointLights[0].Kq", 0.272f);

    unsigned texture = Model::textureFromFile("res/kurac.png");
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
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_BLEND);
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

        view = glm::lookAt(params.position,params.position + params.cameraFront,params.cameraUp);

        phongShader.setMat4("uView", view);
        phongShader.setVec3("uViewPos", params.position);


        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -2.0, 0.0));
        m = glm::scale(m, glm::vec3(12.0, 0.5, 12.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, texture, kockaSpec);



        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -2.0, 0.0));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.02, 0.2, 1.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 0, 1);

        //hud
        DrawHud(hudShader, hudTex);
        phongShader.use();

        //end
        glfwSwapBuffers(window);
        glfwPollEvents();

        FrameEndTime = glfwGetTime();
        params.dt = FrameEndTime - FrameStartTime;
    }

    glfwTerminate();
    return 0;
}

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);

    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}


