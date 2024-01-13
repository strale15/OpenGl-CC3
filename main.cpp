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

    glm::vec3 cameraFront = glm::vec3(-1.0, 0.0, 0.0);
    glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

    glm::vec3 position = glm::vec3(-2.0, 2.0, 0.0);
    glm::vec3 objPos = glm::vec3(0.0, 0.0, 0.0);

    double camYaw = -180;
    double camPitch = 0;

    bool wDown = false;
    bool sDown = false;
    bool aDown = false;
    bool dDown = false;

    bool spaceDown = false;
    bool shiftDown = false;

    bool closeToTerminal = false;
    bool lightOn = true;
    bool flashLightOn = true;

    int imgState = 3;

    float zoom = 100.f;

    bool isWireFrame = false;
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
            glm::vec3 newFront = params->cameraFront;
            newFront.y = 0;
            params->position += 7.2f * newFront * params->dt;
        }
        else
            params->objPos.z += 0.5f * params->dt;
    }
    if (params->sDown)
    {
        if (params->isFps) {
            glm::vec3 newFront = params->cameraFront;
            newFront.y = 0;
            params->position -= 7.2f * newFront * params->dt;
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
    //if (params->spaceDown)
    //{
    //    if (params->isFps)
    //        params->position.y += 4.2 * params->dt;
    //    else
    //        params->objPos.y += 0.5f * params->dt;
    //}
    //if (params->shiftDown)
    //{
    //    if (params->isFps)
    //        params->position.y -= 4.1 * params->dt;
    //    else
    //        params->objPos.y -= 0.5f * params->dt;
    //}

    if (params->isWireFrame) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

    //Gustav
    if (key == GLFW_KEY_F) {
        if (action == GLFW_PRESS) {
            params->flashLightOn = !params->flashLightOn;
        }
    }
    if (key == GLFW_KEY_G) {
        if (action == GLFW_PRESS && params->closeToTerminal) {
            params->lightOn = !params->lightOn;
        }
    }
    if (key == GLFW_KEY_1) {
        if (action == GLFW_PRESS) {
            params->imgState = 1;
        }
    }
    if (key == GLFW_KEY_2) {
        if (action == GLFW_PRESS) {
            params->imgState = 2;
        }
    }
    if (key == GLFW_KEY_3) {
        if (action == GLFW_PRESS) {
            params->imgState = 3;
        }
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS && mode == GLFW_MOD_ALT) {
        params->isWireFrame = true;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS && mode == GLFW_MOD_ALT) {
        params->isWireFrame = false;
    }
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Params* params = (Params*)glfwGetWindowUserPointer(window);
    params->zoom -= static_cast<float>(yoffset * 7000.0 * params->dt);
    params->zoom = glm::clamp(params->zoom, 20.0f, 100.0f);

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
    glfwMakeContextCurrent(window);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursosPosCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetScrollCallback(window, ScrollCallback);

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

    std::vector<float> vertices2 = {
        // Positions      // UVs
        -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,  // Vertex 1
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f,  // Vertex 2
         0.5f,  0.5f, 0.0f, 0.0f, 0.0f,  // Vertex 3

        -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,  // Vertex 1 (Repeated)
         0.5f,  0.5f, 0.0f, 0.0f, 0.0f,  // Vertex 3 (Repeated)
        -0.5f,  0.5f, 0.0f, 1.0f, 0.0f   // Vertex 4
    };
    GameObject* rectangle2 = new GameObject(vertices2, true);

    Model lija("res/low-poly-fox.obj");
    Model statueModel("res/Virgin Mary Statue.obj");

    //Shader phongShader("phong.vert", "phong.frag");
    Shader phongShader("goro.vert", "goro.frag");
    Shader hudShader("hud.vert", "hud.frag");
    Shader twoD("twoD.vert", "twoD.frag");

    phongShader.use();

    glm::mat4 view;
    glm::mat4 projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
    phongShader.setMat4("uProjection", projectionP);

    phongShader.setVec3("uDirLight.Position", 0.0, 5, 0.0);
    phongShader.setVec3("uDirLight.Direction", 0.1, -5, 0.1);
    phongShader.setVec3("uDirLight.Ka", glm::vec3(0.1));
    phongShader.setVec3("uDirLight.Kd", glm::vec3(0.1));
    phongShader.setVec3("uDirLight.Ks", glm::vec3(1.0, 1.0, 1.0));

   /* phongShader.setVec3("uDirLight.Ka", glm::vec3(0));
    phongShader.setVec3("uDirLight.Kd", glm::vec3(0));*/

    

    unsigned hudTex = Model::textureFromFile("res/hudTex.png");
    unsigned kockaDif = Model::textureFromFile("res/container_diffuse.png");
    unsigned kockaSpec = Model::textureFromFile("res/container_specular.png");

    unsigned woodDif = Model::textureFromFile("res/WoodFloor043_2K-PNG_Color.png");
    unsigned woodSpec = Model::textureFromFile("res/WoodFloor043_2K-PNG_Roughness.png");

    unsigned goldDif = Model::textureFromFile("res/Metal042A_2K-PNG_Color.png");
    unsigned goldSpec = Model::textureFromFile("res/Metal042A_2K-PNG_Metalness.png");

    unsigned tilesDif = Model::textureFromFile("res/Tiles074_2K-PNG_Color.png");
    unsigned tilesSpec = Model::textureFromFile("res/tilesSpec.png");

    unsigned slika1Tex = Model::textureFromFile("res/slika1.png");
    unsigned slika2Tex = Model::textureFromFile("res/slika2.png");
    unsigned slika3Tex = Model::textureFromFile("res/slika3.png");

    float slika1Aspc = 776 / 384.f;

    phongShader.setInt("uMaterial.Kd", 0);
    phongShader.setInt("uMaterial.Ks", 1);
    phongShader.setFloat("uMaterial.Shininess", 0.5 * 128);

    glm::mat4 model2 = glm::mat4(1.0f);
    glm::mat4 m(1.0f);
    float currentRot = 0;
    float FrameStartTime = 0;
    float FrameEndTime = 0;
    float rot = 0;
    float scale = 1.0;
    glm::vec3 objPos = glm::vec3(0, 0.6, 0);
    float time = 0.0f;

    Params params;
    glfwSetWindowUserPointer(window, &params);
    glfwSetWindowPos(window, 0, 40);

    glClearColor(0.2, 0.2, 0.6, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
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
        //Map Edge
        glm::vec3 newPos = params.position;
        if (params.position.x > -10 && params.position.x < 10) {
            if (params.position.z >= -1.7 && params.position.z <= 1.7)
                newPos.x = glm::clamp(params.position.x, -10.f, 9.f);
            else
                newPos.x = glm::clamp(params.position.x, -9.f, 9.f);
            newPos.z = glm::clamp(params.position.z, -9.f, 9.f);
        }
        else if (params.position.x > -20 && params.position.x < -10) {
            newPos.x = glm::clamp(params.position.x, -20.f, -10.f);
            newPos.z = glm::clamp(params.position.z, -1.7f, 1.7f);
        }
        else if (params.position.x > -40 && params.position.x < -20) {
            if(params.position.z >= -1.7 && params.position.z <= 1.7)
                newPos.x = glm::clamp(params.position.x, -39.f, -20.f);
            else
                newPos.x = glm::clamp(params.position.x, -39.f, -21.f);
            newPos.z = glm::clamp(params.position.z, -9.f, 9.f);
        }

        params.position = newPos;

        view = glm::lookAt(params.position,params.position + params.cameraFront,params.cameraUp);

        phongShader.setMat4("uView", view);
        phongShader.setVec3("uViewPos", params.position);

        projectionP = glm::perspective(glm::radians(params.zoom), (float)wWidth / (float)wHeight, 0.1f, 100.f);
        phongShader.setMat4("uProjection", projectionP);

        //SCENE
        //------------------------------------------------------------------------------------------------------------
        m = glm::translate(glm::mat4(1.0), objPos);
        m = glm::rotate(m, glm::radians(rot), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(3.8));
        phongShader.setMat4("uModel", m);
        statueModel.Draw(phongShader);

        glm::vec3 direction = objPos - params.position;
        direction.y = 0;
        glm::vec3 camDir2D = params.cameraFront;
        camDir2D.y = 0;
        float dotProduct = glm::dot(glm::normalize(direction), camDir2D);
        float angleRadians = std::acos(dotProduct);
        float angleDegrees = glm::degrees(angleRadians);

        if (angleDegrees > 80) {
            if (glm::distance(objPos, params.position) >= 1.5) {
                glm::vec3 camPos = params.position;
                camPos.y = objPos.y;
                glm::vec3 direction = camPos - objPos;
                direction = glm::normalize(direction);
                rot += 50 * params.dt;
                //objPos += direction * params.dt * 6.f;
            }
            
        }

        //Room1
        //Pod1
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
        m = glm::scale(m, glm::vec3(20, 1.0, 20));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tilesDif,tilesSpec);

        //Krov
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 7.5, 0.0));
        m = glm::scale(m, glm::vec3(20, 1.0, 20));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, woodDif, woodSpec);

        //Zid1
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 3.5, 10.0));
        m = glm::scale(m, glm::vec3(20, 7.0, 1.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, woodDif,woodSpec);

        //Zid2
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 3.5, -10.0));
        m = glm::scale(m, glm::vec3(20, 7.0, 1.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, woodDif, woodSpec);

        //Zid3
        m = glm::translate(glm::mat4(1.0), glm::vec3(10.0, 3.5, 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(20, 7.0, 1.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, woodDif, woodSpec);

        //Small wall1
        m = glm::translate(glm::mat4(1.0), glm::vec3(-10.5, 3.5, 6.0));
        //m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(1.0, 7.0, 8));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, woodDif, woodSpec);

        //Small wall2
        m = glm::translate(glm::mat4(1.0), glm::vec3(-10.5, 3.5, -6.0));
        //m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(1.0, 7.0, 8));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, woodDif, woodSpec);

        //Room2
        //Pod2
        m = glm::translate(glm::mat4(1.0), glm::vec3(-30.0, 0.0, 0.0));
        m = glm::scale(m, glm::vec3(20, 1.0, 20));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tilesDif, tilesSpec);

        m = glm::translate(glm::mat4(1.0), glm::vec3(-30.0, 7.5, 0.0));
        m = glm::scale(m, glm::vec3(20, 1.0, 20));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, goldDif, goldSpec);

        //Zid1
        m = glm::translate(glm::mat4(1.0), glm::vec3(-30.0, 3.5, 10.0));
        m = glm::scale(m, glm::vec3(20, 7.0, 1.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, goldDif, goldSpec);

        //Zid2
        m = glm::translate(glm::mat4(1.0), glm::vec3(-30.0, 3.5, -10.0));
        m = glm::scale(m, glm::vec3(20, 7.0, 1.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, goldDif, goldSpec);

        //Zid3
        m = glm::translate(glm::mat4(1.0), glm::vec3(10.0-30-20, 3.5, 0.0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(20, 7.0, 1.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, goldDif, goldSpec);

        //Small wall1
        m = glm::translate(glm::mat4(1.0), glm::vec3(-10.0-9.5, 3.5, 6.0));
        //m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(1.0, 7.0, 8));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, goldDif, goldSpec);

        //Small wall2
        m = glm::translate(glm::mat4(1.0), glm::vec3(-10.0-9.5, 3.5, -6.0));
        //m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(1.0, 7.0, 8));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, goldDif, goldSpec);

        //Hodnik
        //Pod
        m = glm::translate(glm::mat4(1.0), glm::vec3(-15.0, -0.01, 3.3333333333));
        m = glm::scale(m, glm::vec3(20.0, 1.0, 20.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, tilesDif, tilesSpec);

        //Krov
        m = glm::translate(glm::mat4(1.0), glm::vec3(-15.0, 7.501, 0.0));
        m = glm::scale(m, glm::vec3(20.0, 1.0, 20.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, woodDif, woodSpec);

        //Zid1
        m = glm::translate(glm::mat4(1.0), glm::vec3(-15.0, 3.5, 2.49));
        m = glm::scale(m, glm::vec3(9.5, 7.0, 1.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, woodDif, woodSpec);

        //Zid1
        m = glm::translate(glm::mat4(1.0), glm::vec3(-15.0, 3.5, -2.49));
        m = glm::scale(m, glm::vec3(9.5, 7.0, 1.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, woodDif, woodSpec);

        //Terminal
        glm::vec3 terminalPos = glm::vec3(-10.0 - 9.5 - 1, 1.0, 6.0);
        m = glm::translate(glm::mat4(1.0), terminalPos);
        //m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(1.0, 2.0, 1.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(&phongShader, 1, 1, 1);

        params.closeToTerminal = false;
        float distance = glm::distance(terminalPos, params.position);
        if (distance < 2) {
            params.closeToTerminal = true;
        }

        glm::vec3 zeroVec = glm::vec3(0);
        if (params.flashLightOn) {
            phongShader.setVec3("uSpotlights[0].Ka", 0.0, 0.0, 0.0);
            phongShader.setVec3("uSpotlights[0].Kd", glm::vec3(1.0f, 1.0f, 1.0f) / 1.1f);
            phongShader.setVec3("uSpotlights[0].Ks", glm::vec3(1.0));
        }
        else
        {
            phongShader.setVec3("uSpotlights[0].Ka", zeroVec);
            phongShader.setVec3("uSpotlights[0].Kd", zeroVec);
            phongShader.setVec3("uSpotlights[0].Ks", zeroVec);
        }

        phongShader.setVec3("uSpotlights[0].Position", params.position);
        phongShader.setVec3("uSpotlights[0].Direction", params.cameraFront);
        phongShader.setFloat("uSpotlights[0].InnerCutOff", glm::cos(glm::radians(35.0f)));
        phongShader.setFloat("uSpotlights[0].OuterCutOff", glm::cos(glm::radians(40.0f)));
        phongShader.setFloat("uSpotlights[0].Kc", 1.0);
        phongShader.setFloat("uSpotlights[0].Kl", 0.072f);
        phongShader.setFloat("uSpotlights[0].Kq", 0.012f);

        time += params.dt;  // Increment this over time
        float change = 2;
        glm::vec3 rgbColor = glm::vec3(0);
        if (time < change) {
            rgbColor = glm::vec3(1,0.3,0.3);
        }
        else if(time < change * 2)
        {
            rgbColor = glm::vec3(0.3, 1, 0.3);
        }
        else if (time < change * 3)
        {
            rgbColor = glm::vec3(0.3, 0.3, 1);
        }
        else if (time < change * 4)
        {
            rgbColor = glm::vec3(1, 1, 1);
        }
        else
        {
            time = 0;
        }

        phongShader.setVec3("uSpotlights[1].Position", glm::vec3(0, 6.8, 0));
        phongShader.setVec3("uSpotlights[1].Direction", 0.0, -1.0, 0.0);
        phongShader.setVec3("uSpotlights[1].Ka", 0.0, 0.0, 0.0);
        phongShader.setVec3("uSpotlights[1].Kd", rgbColor *1.1f);
        phongShader.setVec3("uSpotlights[1].Ks", rgbColor*1.f);
        phongShader.setFloat("uSpotlights[1].InnerCutOff", glm::cos(glm::radians(27.0f)));
        phongShader.setFloat("uSpotlights[1].OuterCutOff", glm::cos(glm::radians(36.0f)));
        phongShader.setFloat("uSpotlights[1].Kc", 1.0);
        phongShader.setFloat("uSpotlights[1].Kl", 0.092f);
        phongShader.setFloat("uSpotlights[1].Kq", 0.062f);

        if (params.lightOn) {
            phongShader.setVec3("uPointLights[0].Ka", glm::vec3(0.2,0.2,0.0));
            phongShader.setVec3("uPointLights[0].Kd", glm::vec3(2.0,2.0,0.1));
            phongShader.setVec3("uPointLights[0].Ks", glm::vec3(1.0f));
        }
        else
        {
            phongShader.setVec3("uPointLights[0].Ka", zeroVec);
            phongShader.setVec3("uPointLights[0].Kd", zeroVec);
            phongShader.setVec3("uPointLights[0].Ks", zeroVec);
        }

        phongShader.setVec3("uPointLights[0].Position", glm::vec3(-30.0, 6.0, 0.0));
        phongShader.setFloat("uPointLights[0].Kc", 1.5f);
        phongShader.setFloat("uPointLights[0].Kl", 0.05f);
        phongShader.setFloat("uPointLights[0].Kq", 0.0572f);

        //2D Slike
        twoD.use();

        twoD.setMat4("uView", view);
        twoD.setMat4("uProjection", projectionP);

        //SLIKE
        float ramThick = 0.16;
        float scaleFactor = 2.f;
        float imgRot = 0;
        if (params.imgState == 2)
            imgRot = 180.f;

        //Slika1
        m = glm::translate(glm::mat4(1.0), glm::vec3(-30-9.99+0.5+0.01, 3.5, 0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::rotate(m, glm::radians(imgRot), glm::vec3(0.0, 0.0, 1.0));
        m = glm::scale(m, glm::vec3(1.0, 1.0 * slika1Aspc, 1.0) * scaleFactor);
        twoD.setMat4("uModel", m);
        if(params.imgState == 1)
            rectangle2->Render(&phongShader, slika1Tex);
        else
            rectangle->Render(&phongShader, slika1Tex);

        //Slika2
        m = glm::translate(glm::mat4(1.0), glm::vec3(-30, 3.5, -9.99+0.5+0.01));
        m = glm::rotate(m, glm::radians(0.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::rotate(m, glm::radians(imgRot), glm::vec3(0.0, 0.0, 1.0));
        m = glm::scale(m, glm::vec3(1.0, 1.0, 1.0) * scaleFactor);
        twoD.setMat4("uModel", m);
        if (params.imgState == 1)
            rectangle2->Render(&phongShader, slika2Tex);
        else
            rectangle->Render(&phongShader, slika2Tex);


        //Slika3
        m = glm::translate(glm::mat4(1.0), glm::vec3(-30, 3.5, 9.99-0.5-0.01));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::rotate(m, glm::radians(imgRot), glm::vec3(0.0, 0.0, 1.0));
        m = glm::scale(m, glm::vec3(1.0, 1.0, 1.0) * scaleFactor);
        twoD.setMat4("uModel", m);
        if (params.imgState == 1)
            rectangle2->Render(&phongShader, slika3Tex);
        else
            rectangle->Render(&phongShader, slika3Tex);

        //Ramovi
        //Ram1
        if (params.imgState == 1) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glPointSize(3.0f);
        }
        else if (params.imgState == 2) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        m = glm::translate(glm::mat4(1.0), glm::vec3(-30 - 9.99 + 0.5, 3.5, 0));
        m = glm::rotate(m, glm::radians(90.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::rotate(m, glm::radians(imgRot), glm::vec3(0.0, 0.0, 1.0));
        m = glm::scale(m, glm::vec3(1.0 + ramThick, 1.0 * slika1Aspc + ramThick, 1.0) * scaleFactor);
        twoD.setMat4("uModel", m);
        rectangle->Render(&phongShader, 0.2, 0.16, 0.09);

        //Ram2
        m = glm::translate(glm::mat4(1.0), glm::vec3(-30, 3.5, -9.99 + 0.5));
        m = glm::rotate(m, glm::radians(0.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::rotate(m, glm::radians(imgRot), glm::vec3(0.0, 0.0, 1.0));
        m = glm::scale(m, glm::vec3(1.0 + ramThick, 1.0 + ramThick, 1.0) * scaleFactor);
        twoD.setMat4("uModel", m);
        rectangle->Render(&phongShader, 0.2, 0.16, 0.09);

        //Ram3
        m = glm::translate(glm::mat4(1.0), glm::vec3(-30, 3.5, 9.99 - 0.5));
        m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::rotate(m, glm::radians(imgRot), glm::vec3(0.0, 0.0, 1.0));
        m = glm::scale(m, glm::vec3(1.0+ ramThick, 1.0+ ramThick, 1.0) * scaleFactor);
        twoD.setMat4("uModel", m);
        rectangle->Render(&phongShader, 0.2, 0.16, 0.09);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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