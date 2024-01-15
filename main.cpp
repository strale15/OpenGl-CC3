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
float narutoAspect = 1.469;
float madaraAspect = 1.064;
float tvLength = 0.3 * 16.f;
float tvHegiht = 0.3 * 9.f;

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

    //TV
    bool tvOn = true;
    float offBtnColor = 0.1;
    bool btnColorReached = false;

    bool left1Down = false;
    bool left2Down = false;
    bool right1Down = false;
    bool right2Down = false;

    float xoffset1 = 0;
    float xoffset2 = 0;

    int channel = 1;
    float channelTime = 0;
    int currChannel = 1;

    bool renderPoints = false;
    float clockRot = 0;

    bool phong = true;

    float remoteYrot = 0;
    float remoteZrot = 0;

    float remoteXoffset = 0;
    float remoteYoffset = 0;
    float remoteZoffset = 0;

    bool remoteUp = false;
    bool remoteDown = false;
    bool remoteRight = false;
    bool remoteLeft = false;
    bool remoteHigher = false;
    bool remoteLower = false;
    bool remoteRotLeftY = false;
    bool remoteRotRightY = false;
    bool remoteRotLeftZ = false;
    bool remoteRotRightZ = false;

    float remoteLampAngle = 0;

    float cameraXoffset = 0;
    float cameraYoffset = 0;
    bool isOrtho = false;
    float zoom = 80;

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

    params->clockRot += params->dt*6;
    if (params->clockRot > 360) {
        params->clockRot -= 360;
    }

    //TV
    float remoteSpeed = 4;
    float remoteRotSpeed = 40;
    if (params->remoteUp) {
        params->remoteZoffset += remoteSpeed * params->dt;
        params->remoteZoffset = glm::clamp(params->remoteZoffset, -20.f, 5.f);
    }
    if (params->remoteDown) {
        params->remoteZoffset -= remoteSpeed * params->dt;
        params->remoteZoffset = glm::clamp(params->remoteZoffset, -20.f, 5.f);
    }
    if (params->remoteLeft) {
        params->remoteXoffset += remoteSpeed * params->dt;
        params->remoteZoffset = glm::clamp(params->remoteZoffset, -20.f, 20.f);
    }
    if (params->remoteRight) {
        params->remoteXoffset -= remoteSpeed * params->dt;
        params->remoteZoffset = glm::clamp(params->remoteZoffset, -20.f, 20.f);
    }
    if (params->remoteHigher) {
        params->remoteYoffset += remoteSpeed * params->dt;
        params->remoteYoffset = glm::clamp(params->remoteYoffset, 0.f, 20.f);
    }
    if (params->remoteLower) {
        params->remoteYoffset -= remoteSpeed * params->dt;
        params->remoteYoffset = glm::clamp(params->remoteYoffset, 0.f, 20.f);
    }

    if (params->remoteRotLeftY) {
        params->remoteYrot += remoteRotSpeed * params->dt;
    }
    if (params->remoteRotRightY) {
        params->remoteYrot -= remoteRotSpeed * params->dt;
    }
    if (params->remoteRotLeftZ) {
        params->remoteZrot += remoteRotSpeed * params->dt;
    }
    if (params->remoteRotRightZ) {
        params->remoteZrot -= remoteRotSpeed * params->dt;
    }

    float charSpeed = 2;
    if (params->left1Down) {
        params->xoffset1 += charSpeed * params->dt;
        params->xoffset1 = glm::clamp(params->xoffset1, -10.f, 10.f);
        params->xoffset1 = glm::clamp(params->xoffset1, -((tvLength * 0.9f / 4.f) - 0.33f*narutoAspect), tvLength * 0.9f / 4.f - 0.33f * narutoAspect);
    }
    if (params->right1Down) {
        params->xoffset1 -= charSpeed * params->dt;
        params->xoffset1 = glm::clamp(params->xoffset1, -10.f, 10.f);
        params->xoffset1 = glm::clamp(params->xoffset1, -((tvLength * 0.9f / 4.f) - 0.33f * narutoAspect), tvLength * 0.9f / 4.f - 0.33f * narutoAspect);
    }
    if (params->left2Down) {
        params->xoffset2 += charSpeed * params->dt;
        params->xoffset2 = glm::clamp(params->xoffset2, -((tvLength*0.9f/4.f)-0.43f), tvLength * 0.9f / 4.f - 0.43f);
    }
    if (params->right2Down) {
        params->xoffset2 -= charSpeed * params->dt;
        params->xoffset2 = glm::clamp(params->xoffset2, -((tvLength * 0.9f / 4.f) - 0.43f), tvLength * 0.9f / 4.f - 0.43f);
    }

    if (!params->tvOn) {
        if (params->btnColorReached) {
            params->offBtnColor -= 0.8 * params->dt;
        }
        else
        {
            params->offBtnColor += 0.8 * params->dt;
        }

        if (params->offBtnColor >= 1.0 || params->offBtnColor <= 0) {
            params->btnColorReached = !params->btnColorReached;
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

    //Remote

    if (key == GLFW_KEY_I) {
        if (action == GLFW_PRESS) {
            params->remoteUp = true;
        }
        else if (action == GLFW_RELEASE) {
            params->remoteUp = false;
        }
    }
    if (key == GLFW_KEY_K) {
        if (action == GLFW_PRESS) {
            params->remoteDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->remoteDown = false;
        }
    }
    if (key == GLFW_KEY_J) {
        if (action == GLFW_PRESS) {
            params->remoteLeft = true;
        }
        else if (action == GLFW_RELEASE) {
            params->remoteLeft = false;
        }
    }
    if (key == GLFW_KEY_L) {
        if (action == GLFW_PRESS) {
            params->remoteRight = true;
        }
        else if (action == GLFW_RELEASE) {
            params->remoteRight = false;
        }
    }
    if (key == GLFW_KEY_Y) {
        if (action == GLFW_PRESS) {
            params->remoteHigher = true;
        }
        else if (action == GLFW_RELEASE) {
            params->remoteHigher = false;
        }
    }
    if (key == GLFW_KEY_U) {
        if (action == GLFW_PRESS) {
            params->remoteLower = true;
        }
        else if (action == GLFW_RELEASE) {
            params->remoteLower = false;
        }
    }
    if (key == GLFW_KEY_6) {
        if (action == GLFW_PRESS) {
            params->remoteRotLeftY = true;
        }
        else if (action == GLFW_RELEASE) {
            params->remoteRotLeftY = false;
        }
    }
    if (key == GLFW_KEY_7) {
        if (action == GLFW_PRESS) {
            params->remoteRotRightY = true;
        }
        else if (action == GLFW_RELEASE) {
            params->remoteRotRightY = false;
        }
    }
    if (key == GLFW_KEY_8) {
        if (action == GLFW_PRESS) {
            params->remoteRotLeftZ = true;
        }
        else if (action == GLFW_RELEASE) {
            params->remoteRotLeftZ = false;
        }
    }
    if (key == GLFW_KEY_9) {
        if (action == GLFW_PRESS) {
            params->remoteRotRightZ = true;
        }
        else if (action == GLFW_RELEASE) {
            params->remoteRotRightZ = false;
        }
    }

    //Cam
    if (key == GLFW_KEY_Q) {
        if (action == GLFW_PRESS) {
            params->isOrtho = !params->isOrtho;
        }
    }

    if (key == GLFW_KEY_X) {
        if (action == GLFW_PRESS) {
            params->phong = !params->phong;
        }
    }

    //TV controls
    if (params->remoteLampAngle > 60)
        return;

    if (key == GLFW_KEY_LEFT) {
        if (action == GLFW_PRESS) {
            params->left1Down = true;
        }
        else if (action == GLFW_RELEASE) {
            params->left1Down = false;
        }
    }
    if (key == GLFW_KEY_RIGHT) {
        if (action == GLFW_PRESS) {
            params->right1Down = true;
        }
        else if (action == GLFW_RELEASE) {
            params->right1Down = false;
        }
    }
    if (key == GLFW_KEY_UP) {
        if (action == GLFW_PRESS) {
            params->right2Down = true;
        }
        else if (action == GLFW_RELEASE) {
            params->right2Down = false;
        }
    }
    if (key == GLFW_KEY_DOWN) {
        if (action == GLFW_PRESS) {
            params->left2Down = true;
        }
        else if (action == GLFW_RELEASE) {
            params->left2Down = false;
        }
    }

    if (key == GLFW_KEY_O) {
        if (action == GLFW_PRESS) {
            params->tvOn = true;
        }
    }
    if (key == GLFW_KEY_F) {
        if (action == GLFW_PRESS) {
            params->tvOn = false;
        }
    }

    if (key == GLFW_KEY_1) {
        if (action == GLFW_PRESS) {
            params->channel = 1;
            params->renderPoints = false;
        }
    }
    if (key == GLFW_KEY_2) {
        if (action == GLFW_PRESS) {
            params->renderPoints = true;
        }
    }
    if (key == GLFW_KEY_3) {
        if (action == GLFW_PRESS) {
            params->channel = 2;
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

int main()
{
    HWND console = GetConsoleWindow();
    SetWindowPos(console, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    if (!glfwInit())
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

    if (glewInit() != GLEW_OK)
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

    std::vector<float> circleVert = generateCircleVertices(1, 32);
    GameObject* circle = new GameObject(circleVert, true);

    Model remoteModel("res/tv-remote-control.obj");

    Shader currentShader;
    Shader phongShader("phong.vert", "phong.frag");
    Shader goroShader("goro.vert", "goro.frag");
    Shader hudShader("hud.vert", "hud.frag");
    Shader twoD("twoD.vert", "twoD.frag");
    currentShader = phongShader;

    unsigned hudTex = Model::textureFromFile("res/hudTex.png");
    unsigned laminatDif = Model::textureFromFile("res/laminat.png");
    unsigned laminatSpec = Model::textureFromFile("res/laminatSpec.png");
    unsigned tepihDif = Model::textureFromFile("res/tepih.png");
    unsigned narutoTex = Model::textureFromFile("res/naruto.png");
    unsigned mataraTex = Model::textureFromFile("res/madara4.png");

    glm::mat4 view;
    glm::mat4 projectionP;
    glm::mat4 m(1.0f);
    float currentRot = 0;
    float FrameStartTime = 0;
    float FrameEndTime = 0;

    Params params;
    glfwSetWindowUserPointer(window, &params);

    glClearColor(0.88, 0.88, 0.7, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(3.0);
    glfwWindowHint(GLFW_SAMPLES, 16);
    glEnable(GL_MULTISAMPLE);
    while (!glfwWindowShouldClose(window))
    {
        FrameStartTime = glfwGetTime();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (params.phong) {
            currentShader = phongShader;
        }
        else
        {
            currentShader = goroShader;
        }
        //Shadersetup
        currentShader.use();

        currentShader.setVec3("uDirLight.Position", 0.0, 25, 0.0);
        currentShader.setVec3("uDirLight.Direction", 0.2, -0.4, -0.5);
        currentShader.setVec3("uDirLight.Ka", glm::vec3(0.2));
        currentShader.setVec3("uDirLight.Kd", glm::vec3(0.5));
        currentShader.setVec3("uDirLight.Ks", glm::vec3(1.0, 1.0, 1.0));

        //currentShader.setVec3("uDirLight.Ka", glm::vec3(0));
        //currentShader.setVec3("uDirLight.Kd", glm::vec3(0));

        currentShader.setInt("uMaterial.Kd", 0);
        currentShader.setInt("uMaterial.Ks", 1);
        currentShader.setFloat("uMaterial.Shininess", 0.5 * 128);

        //Loop
        currentShader.use();
        HandleInput(&params);

        //Camera
        projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
        view = glm::lookAt(params.position, params.position + params.cameraFront, params.cameraUp);

        currentShader.setMat4("uProjection", projectionP);
        currentShader.setMat4("uView", view);
        currentShader.setVec3("uViewPos", params.position);

        //SCENE
        //------------------------------------------------------------------------------------------------------------
        float tvColor = 0.6;

        //Pod laminat
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
        //m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(20.0, 1.0, 20.0));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, laminatDif, laminatSpec);

        //Pod tepih
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.001, -0.5));
        //m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(7.0, 1.0, 7.0));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, tepihDif);

        //Stalak1
        m = glm::translate(glm::mat4(1.0), glm::vec3(tvLength / 2 - 0.3, 0.25 + 0.5, 6));
        m = glm::scale(m, glm::vec3(0.4, 0.5, 0.4));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, tvColor, tvColor, tvColor);

        //Stalak2
        m = glm::translate(glm::mat4(1.0), glm::vec3(-tvLength / 2 + 0.3, 0.25 + 0.5, 6));
        m = glm::scale(m, glm::vec3(0.4, 0.5, 0.4));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, tvColor, tvColor, tvColor);

        //TvBase
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, tvHegiht / 2 + 1.0, 6.0));
        m = glm::scale(m, glm::vec3(tvLength, tvHegiht, 0.2));
        currentShader.setMat4("uModel", m);
        simpleCube->Render(&currentShader, tvColor, tvColor, tvColor);

        //Calculate chennel
        if (params.channel != params.currChannel) {
            if (params.channelTime >= 0.5) {
                params.currChannel = params.channel;
                params.channelTime = 0;
            }
            else
            {
                params.channelTime += params.dt;
            }
        }

        //TvScreen
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, tvHegiht / 2 + 1.0, 5.98));
        m = glm::scale(m, glm::vec3(tvLength * 0.9, tvHegiht * 0.9, 0.2));
        currentShader.setMat4("uModel", m);

        float charDist = 0;
        if (!params.tvOn || params.channelTime != 0)
        {
            simpleCube->Render(&currentShader, 0, 0, 0);
        }
        else
        {
            simpleCube->Render(&currentShader, 1, 1, 1);
            charDist = 3.5 - (params.xoffset1 - params.xoffset2 + 1.24523);
        }
        currentShader.setVec3("uSpotlights[0].Position", glm::vec3(0.0, tvHegiht / 2 + 1.0, 5.98));
        currentShader.setVec3("uSpotlights[0].Direction", 0.0, 0.0, -1.0);
        currentShader.setVec3("uSpotlights[0].Ka", 0.0, 0.0, 0.0);
        currentShader.setVec3("uSpotlights[0].Kd", glm::vec3(0.8f)* charDist);
        currentShader.setVec3("uSpotlights[0].Ks", glm::vec3(1.0)* charDist/3.f);
        currentShader.setFloat("uSpotlights[0].InnerCutOff", glm::cos(glm::radians(75.0f)));
        currentShader.setFloat("uSpotlights[0].OuterCutOff", glm::cos(glm::radians(81.0f)));
        currentShader.setFloat("uSpotlights[0].Kc", 1.5);
        currentShader.setFloat("uSpotlights[0].Kl", 0.092f);
        currentShader.setFloat("uSpotlights[0].Kq", 0.032f);

        //TvOnBtn
        // m = glm::translate(glm::mat4(1.0), glm::vec3(tvLength/2-0.05*6, 1.0+0.27/4, 6.0-0.1));
        if (params.tvOn) {
            m = glm::translate(glm::mat4(1.0), glm::vec3(tvLength/2-0.05*10, 1.0+0.27/4, 6.0-0.1));
            m = glm::scale(m, glm::vec3(0.05, 0.05, 0.1));
            currentShader.setMat4("uModel", m);
            simpleCube->Render(&currentShader, 0.79, 0.24, 0.84);
        }
        else
        {
            m = glm::translate(glm::mat4(1.0), glm::vec3(tvLength / 2 - 0.05 * 8, 1.0 + 0.27 / 4, 6.0 - 0.1));
            m = glm::scale(m, glm::vec3(0.05, 0.05, 0.1));
            currentShader.setMat4("uModel", m);
            simpleCube->Render(&currentShader, params.offBtnColor, params.offBtnColor, params.offBtnColor);
        }

        //Lampica
        glm::vec3 lampPos = glm::vec3(tvLength / 2 - 0.05 * 6, 1.0 + 0.27 / 4, 6.0 -0.025-0.1);
        m = glm::translate(glm::mat4(1.0), lampPos);
        m = glm::scale(m, glm::vec3(0.05, 0.05, 0.05));
        currentShader.setMat4("uModel", m);
        if (params.tvOn) {
            simpleCube->Render(&currentShader, 1, 1, 1);

            if (params.channelTime != 0) {
                currentShader.setVec3("uPointLights[0].Ka", glm::vec3(0.1, 0.1, 0.0));
                currentShader.setVec3("uPointLights[0].Kd", glm::vec3(8.f, 8.0f, 0.0));
                currentShader.setVec3("uPointLights[0].Ks", glm::vec3(1.0, 1.0f, 0));
            }
            else
            {
                currentShader.setVec3("uPointLights[0].Ka", glm::vec3(0.1, 0.1, 0.0));
                currentShader.setVec3("uPointLights[0].Kd", glm::vec3(0.0, 8.0f, 0.0));
                currentShader.setVec3("uPointLights[0].Ks", glm::vec3(0, 1.0f, 0));
            }

        }
        else
        {
            simpleCube->Render(&currentShader, 0, 1, 0);
            currentShader.setVec3("uPointLights[0].Ka", glm::vec3(0.0, 0.0, 0.0));
            currentShader.setVec3("uPointLights[0].Kd", glm::vec3(0.0, 0.0f, 0.0));
            currentShader.setVec3("uPointLights[0].Ks", glm::vec3(0, 0.0f, 0));
        }

        glm::vec3 lampPos2 = lampPos;
        lampPos2.z -= 0.03;
        currentShader.setVec3("uPointLights[0].Position", lampPos2);
        currentShader.setFloat("uPointLights[0].Kc", 5.0f);
        currentShader.setFloat("uPointLights[0].Kl", 15.0f);
        currentShader.setFloat("uPointLights[0].Kq", 25.f);

        //Remote
        glm::vec3 remotePos = glm::vec3(0.0 + params.remoteXoffset, 0.65 + params.remoteYoffset, 1.0 + params.remoteZoffset);
        m = glm::translate(glm::mat4(1.0), remotePos);
        m = glm::rotate(m, glm::radians(180.f+ params.remoteYrot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::rotate(m, glm::radians(params.remoteZrot), glm::vec3(1.0, 0.0, 0.0));
        m = glm::scale(m, glm::vec3(0.07, 0.07, 0.07));
        currentShader.setMat4("uModel", m);
        remoteModel.Draw(currentShader);

        //Angle calculations
        glm::vec3 remoteForward = -glm::vec3(m[2]);
        remoteForward = glm::normalize(remoteForward);
        glm::vec3 direction = lampPos - (remotePos + remoteForward*0.66f);

        float dotProduct = glm::dot(glm::normalize(direction), remoteForward);
        dotProduct = glm::clamp(dotProduct, -1.f, 1.f);
        float angleRadians = std::acos(dotProduct);
        float angleDegrees = glm::degrees(angleRadians);
        params.remoteLampAngle = angleDegrees;

        //2D Stvari
        twoD.use();
        twoD.setMat4("uProjection", projectionP);
        twoD.setMat4("uView", view);

        if (params.tvOn && params.currChannel == 1 && params.channelTime == 0) {
            //Clock
            if (params.renderPoints) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            }

            m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, tvHegiht / 2 + 1.0, 5.98 - 0.101));
            m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(1.0));
            twoD.setMat4("uModel", m);
            circle->Render(&twoD, 1, 1, 0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            //Big pointer
            float pointLen = 0.9;
            m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, tvHegiht / 2 + 1.0, 5.98 - 0.102));
            m = glm::rotate(m, glm::radians(params.clockRot - 180), glm::vec3(0.0, 0.0, 1.0));
            m = glm::translate(m, -glm::vec3(0.0, tvHegiht / 2 + 1.0, 5.98 - 0.102));

            m = glm::translate(m, glm::vec3(0.0, tvHegiht / 2 + 1.0 - pointLen / 2, 5.98 - 0.102));
            m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(0.02, pointLen, 1.0));
            twoD.setMat4("uModel", m);
            rectangle->Render(&twoD, 1, 0, 0);

            //Small pointer
            pointLen = 0.7;
            m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, tvHegiht / 2 + 1.0 - pointLen / 2, 5.98 - 0.1015));
            m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(0.02, pointLen, 1.0));
            twoD.setMat4("uModel", m);
            rectangle->Render(&twoD, 0, 0, 1);

        }
        else if (params.tvOn && params.currChannel == 2 && params.channelTime == 0) {
            //NarutoXMadara
            //Naruto
            m = glm::translate(glm::mat4(1.0), glm::vec3(-(tvLength * 0.9f / 4.f) + params.xoffset1, tvHegiht / 2 + 1.0, 5.98 - 0.1019));
            m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(1.0 * narutoAspect, 1.0, 1.0) / 1.5f);
            twoD.setMat4("uModel", m);
            rectangle->Render(&twoD, narutoTex);

            //Madara
            m = glm::translate(glm::mat4(1.0), glm::vec3((tvLength * 0.9f / 4.f) + params.xoffset2, tvHegiht / 2 + 1.0, 5.98 - 0.1019));
            m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
            m = glm::scale(m, glm::vec3(1.3, 1.3 * madaraAspect, 1.0) / 1.5f);
            twoD.setMat4("uModel", m);
            rectangle->Render(&twoD, mataraTex);
        }

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