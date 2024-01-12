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
#include <math.h>

#include "shader.hpp"
#include "model.hpp"

#include "GameObject.cpp"

const unsigned int wWidth = 1920;
const unsigned int wHeight = 1080;

struct Params {
    float dt = 0;
    bool wDown = false;
    bool sDown = false;
    bool aDown = false;
    bool dDown = false;

    bool volumeUp = false;
    bool volumeDown = false;

    bool antennaUp = false;
    bool antennaDown = false;

    bool antennaRotLeft = false;
    bool antennaRotRight = false;

    bool radioOn = false;
    bool amSliderLeft = false;
    bool amSliderRight = false;
    bool knobTurningLeft = false;
    bool knobTurningRight = false;
    bool isOrtho = false;
    bool antennaSpinLeft = false;
    bool antennaSpinRight = false;
    float antennaSpin = 0;
    float frequency = 0;
    float knobRot = 0;
    float amSlectorXPos = 0;
    float antennaScaleF = 0;
    float antenaRot = 0;

    bool downDown = false;
    bool upDown = false;
    bool rightDown = false;
    bool leftDown = false;
    bool spaceDown = false;
    bool shiftDown = false;

    float mouseDeltaX = 0;
    float mouseDeltaY = 0;
    float lastMouseX = 0;
    float lastMouseY = 0;

    float zoom = 100;
    float volume = 0.1; //goes from 0.1 to 4

    bool isWireframe = false;

    glm::vec3 camPos = glm::vec3(0, 0, 15);
    glm::vec3 objPos = glm::vec3(0, 0, 0);
};

std::vector<float> generateCylinderVertices(int sides, float radius, float height);
void renderNets(Shader& phongShader, GameObject* simpleCube);

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    Params* params = (Params*)glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_W) {
        if (action == GLFW_PRESS) {
            params->wDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->wDown = false;
        }
    }
    if (key == GLFW_KEY_A) {
        if (action == GLFW_PRESS) {
            params->aDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->aDown = false;
        }
    }
    if (key == GLFW_KEY_S) {
        if (action == GLFW_PRESS) {
            params->sDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->sDown = false;
        }
    }
    if (key == GLFW_KEY_D) {
        if (action == GLFW_PRESS) {
            params->dDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->dDown = false;
        }
    }
    if (key == GLFW_KEY_LEFT) {
        if (action == GLFW_PRESS) {
            params->leftDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->leftDown = false;
        }
    }
    if (key == GLFW_KEY_RIGHT) {
        if (action == GLFW_PRESS) {
            params->rightDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->rightDown = false;
        }
    }
    if (key == GLFW_KEY_UP) {
        if (action == GLFW_PRESS) {
            params->upDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->upDown = false;
        }
    }
    if (key == GLFW_KEY_DOWN) {
        if (action == GLFW_PRESS) {
            params->downDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->downDown = false;
        }
    }
    if (key == GLFW_KEY_SPACE) {
        if (action == GLFW_PRESS) {
            params->spaceDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->spaceDown = false;
        }
    }
    if (key == GLFW_KEY_LEFT_SHIFT) {
        if (action == GLFW_PRESS) {
            params->shiftDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->shiftDown = false;
        }
    }
    if (key == GLFW_KEY_L) {
        if (action == GLFW_PRESS) {
            params->isWireframe = !params->isWireframe;
        }
    }
    if (key == GLFW_KEY_R) {
        if (action == GLFW_PRESS) {
            std::cout << "glm::vec3(" << params->objPos.x << "," << params->objPos.y << "," << params->objPos.z << ")" << std::endl;
        }
    }
    if (key == GLFW_KEY_I) {
        if (action == GLFW_PRESS) {
            params->volumeDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->volumeDown = false;
        }
    }
    if (key == GLFW_KEY_O) {
        if (action == GLFW_PRESS) {
            params->volumeUp = true;
        }
        else if (action == GLFW_RELEASE) {
            params->volumeUp = false;
        }
    }
    if (key == GLFW_KEY_Y) {
        if (action == GLFW_PRESS) {
            params->knobTurningLeft = true;
        }
        else if (action == GLFW_RELEASE) {
            params->knobTurningLeft = false;
        }
    }
    if (key == GLFW_KEY_U) {
        if (action == GLFW_PRESS) {
            params->knobTurningRight = true;
        }
        else if (action == GLFW_RELEASE) {
            params->knobTurningRight = false;
        }
    }
    if (key == GLFW_KEY_Q) {
        if (action == GLFW_PRESS) {
            params->radioOn = !params->radioOn;
        }
    }
    if (key == GLFW_KEY_N) {
        if (action == GLFW_PRESS) {
            params->amSliderLeft = true;
        }
        else if (action == GLFW_RELEASE) {
            params->amSliderLeft = false;
        }
    }
    if (key == GLFW_KEY_M) {
        if (action == GLFW_PRESS) {
            params->amSliderRight = true;
        }
        else if (action == GLFW_RELEASE) {
            params->amSliderRight = false;
        }
    }
    if (key == GLFW_KEY_G) {
        if (action == GLFW_PRESS) {
            params->antennaUp = true;
        }
        else if (action == GLFW_RELEASE) {
            params->antennaUp = false;
        }
    }
    if (key == GLFW_KEY_B) {
        if (action == GLFW_PRESS) {
            params->antennaDown = true;
        }
        else if (action == GLFW_RELEASE) {
            params->antennaDown = false;
        }
    }
    if (key == GLFW_KEY_H) {
        if (action == GLFW_PRESS) {
            params->antennaRotLeft = true;
        }
        else if (action == GLFW_RELEASE) {
            params->antennaRotLeft = false;
        }
    }
    if (key == GLFW_KEY_J) {
        if (action == GLFW_PRESS) {
            params->antennaRotRight = true;
        }
        else if (action == GLFW_RELEASE) {
            params->antennaRotRight = false;
        }
    }
    if (key == GLFW_KEY_X) {
        if (action == GLFW_PRESS) {
            params->isOrtho = !params->isOrtho;
        }
    }
    if (key == GLFW_KEY_C) {
        if (action == GLFW_PRESS) {
            params->antennaSpinLeft = true;
        }
        else if (action == GLFW_RELEASE) {
            params->antennaSpinLeft = false;
        }
    }
    if (key == GLFW_KEY_V) {
        if (action == GLFW_PRESS) {
            params->antennaSpinRight = true;
        }
        else if (action == GLFW_RELEASE) {
            params->antennaSpinRight = false;
        }
    }

    bool IsDown = action == GLFW_PRESS || action == GLFW_REPEAT;
    switch (key) {
    case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    }
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Params* params = (Params*)glfwGetWindowUserPointer(window);
    params->zoom -= static_cast<float>(yoffset * 7000.0 * params->dt);
    params->zoom = glm::clamp(params->zoom, 20.0f, 100.0f);

}

static void CursosPosCallback(GLFWwindow* window, double xPos, double yPos) {
    Params* params = (Params*)glfwGetWindowUserPointer(window);

    params->mouseDeltaX = xPos - params->lastMouseX;
    params->lastMouseX = xPos;

    params->mouseDeltaY = yPos - params->lastMouseY;
    params->lastMouseY = yPos;

}

static void HandleInput(Params* params) {
    if (params->wDown || params->sDown || params->aDown || params->dDown) {

        glm::vec3 camFront = glm::normalize(glm::vec3(0) - params->camPos);
        glm::vec3 right = glm::normalize(glm::cross(camFront, glm::vec3(0, 1, 0)));
        float pitchAngle = glm::degrees(asin(camFront.y));


        float orbitSpeed = 100 * params->dt; 
        float orbitAngleX = 0.0f;
        float orbitAngleY = 0.0f;

        if (params->wDown && pitchAngle > -80) {
            orbitAngleX += orbitSpeed / 2;
        }
        if (params->sDown && pitchAngle < 80) {
            orbitAngleX -= orbitSpeed / 2;
        }
        if (params->aDown) {
            orbitAngleY -= orbitSpeed;
        }
        if (params->dDown) {
            orbitAngleY += orbitSpeed;
        }

        glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), glm::radians(orbitAngleX), right);

        glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(orbitAngleY), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::vec4 rotatedPosition = glm::vec4(params->camPos, 1.0f) * rotateX * rotateY;
        params->camPos = glm::vec3(rotatedPosition);
    }

    if (params->leftDown) {
        params->objPos.x -= 3 * params->dt;  // Move left along the X axis
    }

    if (params->rightDown) {
        params->objPos.x += 3 * params->dt;  // Move right along the X axis
    }

    if (params->upDown) {
        params->objPos.z -= 3 * params->dt;  // Move forward along the Z axis
    }

    if (params->downDown) {
        params->objPos.z += 3 * params->dt;  // Move backward along the Z axis
    }

    if (params->spaceDown) {
        params->objPos.y += 3 * params->dt;  // Move backward along the Z axis
    }

    if (params->shiftDown) {
        params->objPos.y -= 3 * params->dt;  // Move backward along the Z axis
    }

    if (params->volumeUp) {
        if(params->volume <= 4)
            params->volume += 2 * params->dt;  // Move backward along the Z axis
    }

    if (params->volumeDown) {
        if (params->volume > 0.1)
            params->volume -= 2 * params->dt;  // Move backward along the Z axis
    }

    if (params->isWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (params->knobTurningLeft) {
        params->knobRot -= 100 * params->dt;
        params->frequency += 0.5 * params->dt;
        params->frequency = glm::clamp(params->frequency, 0.0f, 3.6f);
    }
    else if (params->knobTurningRight) {
        params->knobRot += 100 * params->dt;
        params->frequency -= 0.5 * params->dt;
        params->frequency = glm::clamp(params->frequency, 0.0f, 3.6f);
    }

    if (params->amSliderLeft) {
        params->amSlectorXPos -= 1 * params->dt;
        params->amSlectorXPos = glm::clamp(params->amSlectorXPos, 0.0f, 0.36f/2 + 0.02f);
    } else if (params->amSliderRight) {
        params->amSlectorXPos += 1 * params->dt;
        params->amSlectorXPos = glm::clamp(params->amSlectorXPos, 0.0f, 0.36f/2 + 0.02f);
    }

    if (params->antennaUp) {
        params->antennaScaleF += 1 * params->dt;
        params->antennaScaleF = glm::clamp(params->antennaScaleF, 0.0f, 4.0f);
    }
    else if (params->antennaDown) {
        params->antennaScaleF -= 1 * params->dt;
        params->antennaScaleF = glm::clamp(params->antennaScaleF, 0.0f, 4.0f);
    }

    if (params->antennaRotLeft) {
        params->antenaRot += 50 * params->dt;
        params->antenaRot = glm::clamp(params->antenaRot, -60.0f, 60.0f);
    }
    else if (params->antennaRotRight) {
        params->antenaRot -= 50 * params->dt;
        params->antenaRot = glm::clamp(params->antenaRot, -60.0f, 60.0f);
    }

    if (params->antennaSpinLeft) {
        params->antennaSpin += 50 * params->dt;
    }
    else if (params->antennaSpinRight) {
        params->antennaSpin -= 50 * params->dt;
    }



}

static void DrawHud(Shader &hudShader, unsigned hudTex) {
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
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursosPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    if (glewInit() !=GLEW_OK)
    {
        std::cout << "GLEW fail! :(\n" << std::endl;
        return -3;
    }

    //START
    Params params;
    glfwSetWindowUserPointer(window, &params);
    glfwSetWindowPos(window, 0, 40);

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

    std::vector<float> cylVertices = generateCylinderVertices(32, 2, 1);
    GameObject* simpleCyl = new GameObject(cylVertices);

    Model lija("res/low-poly-fox.obj");

    Shader phongShader("phong.vert", "phong.frag");
    Shader hudShader("hud.vert", "hud.frag");


    phongShader.use();
    glm::mat4 model = glm::mat4(1.0f);
    phongShader.setMat4("uModel", model);

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    phongShader.setMat4("uView", view);
    phongShader.setVec3("uViewPos", 0.0, 0.0, 5.0);

    glm::mat4 projectionP = glm::perspective(glm::radians(45.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
    phongShader.setMat4("uProjection", projectionP);


    phongShader.setVec3("uDirLight.Position", 0.0, 20, 0.0);
    phongShader.setVec3("uDirLight.Direction", 0.1, -5, 0.1);
    phongShader.setVec3("uDirLight.Ka", glm::vec3(0.5));
    phongShader.setVec3("uDirLight.Kd", glm::vec3(0.5));
    phongShader.setVec3("uDirLight.Ks", glm::vec3(10.f));

    unsigned hudTex = Model::textureFromFile("res/hudTex.png");
    unsigned wood = Model::textureFromFile("res/wood.jpg");
    unsigned volumePointer = Model::textureFromFile("res/pointer.png");
    unsigned amFmTex = Model::textureFromFile("res/AM-FM.png");
    unsigned amFmBtnTex = Model::textureFromFile("res/am-fmB.png");
    unsigned knobTex = Model::textureFromFile("res/knob.png");
    unsigned howToUseTex = Model::textureFromFile("res/uputstvo.png");

    phongShader.setInt("uMaterial.Kd", 0);
    phongShader.setInt("uMaterial.Ks", 1);
    phongShader.setFloat("uMaterial.Shininess", 0.25 * 128);

    glm::mat4 model2 = glm::mat4(1.0f);
    glm::mat4 m(1.0f);
    float currentRot = 0;
    float FrameStartTime = 0;
    float FrameEndTime = 0;
    float vibroCoef = 0; // goes from 0 to 1
    bool increasing = true;
    glClearColor(0.2, 0.2, 0.6, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
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

        //Camera
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::lookAt(params.camPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        phongShader.setMat4("uView", view);
        phongShader.setVec3("uViewPos", params.camPos);

        if (params.isOrtho)
            projectionP = glm::ortho(-16.f,16.f,-9.f,9.f, 0.1f, 100.f);
        else
            projectionP = glm::perspective(glm::radians(params.zoom), (float)wWidth / (float)wHeight, 0.1f, 100.0f);

        phongShader.setMat4("uProjection", projectionP);

        //Scene

        //RadioBase
        m = glm::translate(glm::mat4(1.0), glm::vec3(0));
        m = glm::scale(m, glm::vec3(9,5,3));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, wood);

        //HowToUse
        m = glm::translate(glm::mat4(1.0), glm::vec3(0,0,-0.2));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(9-0.15, 5-0.15, 3-0.15));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, howToUseTex);

        //Slider
        float scaleFactorX = params.volume;
        float translationAdjustmentX = (1.0f - scaleFactorX) / 2.0f;

        m = glm::translate(glm::mat4(1.0), glm::vec3(-3.55963 - translationAdjustmentX, -1.23988-1, 1.5));
        m = glm::scale(m, glm::vec3(scaleFactorX, 0.2, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0,1,0);
        
        //SliderPoint
        m = glm::translate(glm::mat4(1.0), glm::vec3(-3.13969 - translationAdjustmentX*2, -1.23988+0.2-1, 1.5));
        m = glm::scale(m, glm::vec3(0.2, 0.2, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, volumePointer);

        //Membrane
        vibroCoef = glm::clamp(vibroCoef, 0.0f, 1.0f);
        if (increasing) {
            vibroCoef += params.volume * params.dt * 1.1f;

            if (vibroCoef >= 1.0f) {
                vibroCoef = 1.0f;
                increasing = false;
            }
        }
        else {
            vibroCoef -= params.volume * params.dt * 1.1f;

            if (vibroCoef <= 0.0f) {
                vibroCoef = 0.0f;
                increasing = true;
            }
        }

        float maxScale = 0.2 + params.volume/7.7;
        float minScale = 0.2;
        float scale = minScale + vibroCoef * (maxScale - minScale);

        if (params.volume <= 0.11 || !params.radioOn) {
            scale = minScale;
        }

        m = glm::translate(glm::mat4(1.0), glm::vec3(2.71993, 0.480017, 1.2));
        m = glm::scale(m, glm::vec3(scale, scale, 0.4));
        phongShader.setMat4("uModel", m);
        simpleCyl->Render(&phongShader, 0,0,1);

        //Net
        renderNets(phongShader, simpleCube);

        //FmScale
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.23995, 1.88003, 1.35));
        m = glm::scale(m, glm::vec3(4, 0.4, 0.4));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, amFmTex);

        //FmKazaljka
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.23995-1.8+params.frequency, 1.77, 1.37));
        m = glm::scale(m, glm::vec3(0.05, 0.4, 0.4));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1,0,0);

        //FmKnob
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.23995 - 0.9, 0.2, 1.37));
        m = glm::rotate(m, glm::radians(params.knobRot), glm::vec3(0.0, 0.0, 1.0));
        m = glm::scale(m, glm::vec3(0.4, 0.4, 0.4));
        phongShader.setMat4("uModel", m);
        simpleCyl->Render(&phongShader, knobTex);

        //OnBtn
        float zoffset = 0;
        if (params.radioOn)
            zoffset = 0.12;
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.23995 - 2.5, 1.77, 1.2 - zoffset));
        m = glm::scale(m, glm::vec3(0.2, 0.2, 1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 0, 0);

        //OnLed
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.23995 - 2.5, 1.77 - 0.25, 1.1));
        m = glm::scale(m, glm::vec3(0.2, 0.2, 1));
        phongShader.setMat4("uModel", m);
        if(params.radioOn)
            simpleCube->Render(&phongShader, 0, 1, 0);
        else
            simpleCube->Render(&phongShader, 1, 1, 1);

        glm::vec3 radioLightD;
        glm::vec3 radioLightA;
        if (params.radioOn) {
            radioLightD = glm::vec3(0.0f, 2.0f, 0.0f);
            radioLightA = glm::vec3(0.2f, 0.2f, 0.2f);
        }
        else {
            radioLightD = glm::vec3(0.0f, 0.0f, 0.0f);
            radioLightA = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        phongShader.setVec3("uPointLights[0].Position", glm::vec3(-1.23995 - 2.5, 1.77 - 0.25, 1.1));
        phongShader.setVec3("uPointLights[0].Ka", radioLightA);
        phongShader.setVec3("uPointLights[0].Kd", radioLightD);
        phongShader.setVec3("uPointLights[0].Ks", glm::vec3(0.0f));
        phongShader.setFloat("uPointLights[0].Kc", 1.5f);
        phongShader.setFloat("uPointLights[0].Kl", 1.0f);
        phongShader.setFloat("uPointLights[0].Kq", 0.272f);

        //AmFm Ind
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.23995 - 1.8, 1.77 - 0.35, 1.05));
        m = glm::scale(m, glm::vec3(0.4, 0.2, 1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, amFmBtnTex);

        //AmFm Select
        m = glm::translate(glm::mat4(1.0), glm::vec3(-1.23995 - 1.8 - 0.1 + + params.amSlectorXPos, 1.77 - 0.35, 1.1));
        m = glm::scale(m, glm::vec3(0.18, 0.18, 1));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1,1,1);

        //Antena
        float translationAdjustmentY = (1.0f - params.antennaScaleF) / 2.0f;

        m = glm::translate(glm::mat4(1.0), glm::vec3(3.40001, 2.5, 0));
        m = glm::rotate(m, glm::radians(params.antenaRot), glm::vec3(0.0, 0.0, 1.0));
        m = glm::translate(m, -glm::vec3(3.40001, 2.5, 0));

        m = glm::translate(m, glm::vec3(3.40001, 2.8 - translationAdjustmentY, 0));
        m = glm::rotate(m, glm::radians(params.antennaSpin), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.4, params.antennaScaleF, 0.4));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0.6, 0.6, 0.6);

        //Antenna light
        glm::vec4 topPoint = m * glm::vec4(0.0f, 0.5f, 0.0f, 1.0f);
        glm::vec4 bottomPoint = m * glm::vec4(0.0f, -0.5f, 0.0f, 1.0f);

        topPoint /= topPoint.w;
        bottomPoint /= bottomPoint.w;
        glm::vec3 topP = glm::vec3(topPoint);
        glm::vec3 bottomP = glm::vec3(bottomPoint);
        glm::vec3 localUp = bottomP - topP;
        localUp = glm::normalize(localUp);

        //Platform for spotlight
        m = glm::translate(glm::mat4(1.0), glm::vec3(0,20,0));
        m = glm::scale(m, glm::vec3(50.f,0.1,50.f));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1,0,0);

        phongShader.setVec3("uSpotlights[0].Position", topP);
        phongShader.setVec3("uSpotlights[0].Direction", -localUp);
        phongShader.setVec3("uSpotlights[0].Ka", glm::vec3(2.0f, 1.0f, 0.0f));
        phongShader.setVec3("uSpotlights[0].Kd", glm::vec3(2.0f, 1.0f, 0.0f));
        phongShader.setVec3("uSpotlights[0].Ks", glm::vec3(50.0f));
        phongShader.setFloat("uSpotlights[0].InnerCutOff", glm::cos(glm::radians(15.0f)));
        phongShader.setFloat("uSpotlights[0].OuterCutOff", glm::cos(glm::radians(25.0f)));
        phongShader.setFloat("uSpotlights[0].Kc", 1.0);
        phongShader.setFloat("uSpotlights[0].Kl", 0.05f);
        phongShader.setFloat("uSpotlights[0].Kq", 0.022f);


        //Hud
        DrawHud(hudShader, hudTex);

        //end
        glfwSwapBuffers(window);
        glfwPollEvents();

        FrameEndTime = glfwGetTime();
        params.dt = FrameEndTime - FrameStartTime;
    }

    glfwTerminate();
    return 0;
}

std::vector<float> generateCylinderVertices(int sides, float radius, float height) {
    std::vector<float> cylinderVertices;

    for (int i = 0; i < sides; ++i) {
        float theta1 = static_cast<float>(i) * (2.0f * static_cast<float>(M_PI) / sides);
        float theta2 = static_cast<float>(i + 1) * (2.0f * static_cast<float>(M_PI) / sides);

        // Bottom cap vertices
        cylinderVertices.insert(cylinderVertices.end(), {
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.5f,
            radius * cos(theta2), radius * sin(theta2), 0.0f, 0.0f, 0.0f, -1.0f, 0.5f + 0.5f * cos(theta2), 0.5f + 0.5f * sin(theta2),
            radius * cos(theta1), radius * sin(theta1), 0.0f, 0.0f, 0.0f, -1.0f, 0.5f + 0.5f * cos(theta1), 0.5f + 0.5f * sin(theta1)
            });

        // Top cap vertices
        cylinderVertices.insert(cylinderVertices.end(), {
            0.0f, 0.0f, height, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f,
            radius * cos(theta1), radius * sin(theta1), height, 0.0f, 0.0f, 1.0f, 0.5f + 0.5f * cos(theta1), 0.5f + 0.5f * sin(theta1),
            radius * cos(theta2), radius * sin(theta2), height, 0.0f, 0.0f, 1.0f, 0.5f + 0.5f * cos(theta2), 0.5f + 0.5f * sin(theta2)
            });

        // Side face vertices (reversed order for correct face direction)
        cylinderVertices.insert(cylinderVertices.end(), {
            radius * cos(theta2), radius * sin(theta2), height, cos(theta2), sin(theta2), 0.0f, static_cast<float>(i + 1) / sides, 1.0f,
            radius * cos(theta1), radius * sin(theta1), height, cos(theta1), sin(theta1), 0.0f, static_cast<float>(i) / sides, 1.0f,
            radius * cos(theta1), radius * sin(theta1), 0.0f, cos(theta1), sin(theta1), 0.0f, static_cast<float>(i) / sides, 0.0f
            });

        cylinderVertices.insert(cylinderVertices.end(), {
            radius * cos(theta1), radius * sin(theta1), 0.0f, cos(theta1), sin(theta1), 0.0f, static_cast<float>(i) / sides, 0.0f,
            radius * cos(theta2), radius * sin(theta2), 0.0f, cos(theta2), sin(theta2), 0.0f, static_cast<float>(i + 1) / sides, 0.0f,
            radius * cos(theta2), radius * sin(theta2), height, cos(theta2), sin(theta2), 0.0f, static_cast<float>(i + 1) / sides, 1.0f
            });
    }

    return cylinderVertices;
}

void renderNets(Shader& phongShader, GameObject* simpleCube) {
    glm::mat4 m;

    // Vertical nets
    for (int i = 0; i < 5; ++i) {
        m = glm::translate(glm::mat4(1.0), glm::vec3(2.71993 + 0.3 * i, 0.480017, 1.6));
        m = glm::scale(m, glm::vec3(0.05, 3, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 1, 1);
    }

    for (int i = 0; i < 5; ++i) {
        m = glm::translate(glm::mat4(1.0), glm::vec3(2.71993 - 0.3 * i, 0.480017, 1.6));
        m = glm::scale(m, glm::vec3(0.05, 3, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 1, 1);
    }

    // Horizontal nets
    for (int i = 0; i < 5; ++i) {
        m = glm::translate(glm::mat4(1.0), glm::vec3(2.71993, 0.480017 + 0.3 * i, 1.6));
        m = glm::scale(m, glm::vec3(3, 0.05, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 1, 1);
    }

    for (int i = 0; i < 5; ++i) {
        m = glm::translate(glm::mat4(1.0), glm::vec3(2.71993, 0.480017 - 0.3 * i, 1.6));
        m = glm::scale(m, glm::vec3(3, 0.05, 0.2));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 0, 1, 1);
    }
}


