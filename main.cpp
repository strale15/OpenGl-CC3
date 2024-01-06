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

struct Params {
    float dt = 0;
    bool wDown = false;
    bool sDown = false;
    bool aDown = false;
    bool dDown = false;

    float mouseDeltaX = 0;
    float mouseDeltaY = 0;
    float lastMouseX = 0;
    float lastMouseY = 0;

    glm::vec3 camPos = glm::vec3(0, 0, 15);
};

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    Params* params = (Params*)glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        params->wDown = true;
    }
    if (key == GLFW_KEY_W && action == GLFW_RELEASE)
    {
        params->wDown = false;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        params->aDown = true;
    }
    if (key == GLFW_KEY_A && action == GLFW_RELEASE)
    {
        params->aDown = false;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        params->sDown = true;
    }
    if (key == GLFW_KEY_S && action == GLFW_RELEASE)
    {
        params->sDown = false;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        params->dDown = true;
    }
    if (key == GLFW_KEY_D && action == GLFW_RELEASE)
    {
        params->dDown = false;
    }


    bool IsDown = action == GLFW_PRESS || action == GLFW_REPEAT;
    switch (key) {
    case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    }
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
        float orbitSpeed = 100 * params->dt; // Adjust the orbit speed as needed

        // Calculate the orbiting angle based on key presses
        float orbitAngleX = 0.0f;
        float orbitAngleY = 0.0f;

        if (params->wDown) {
            orbitAngleX -= orbitSpeed/2;
        }
        if (params->sDown) {
            orbitAngleX += orbitSpeed/2;
        }
        if (params->aDown) {
            orbitAngleY -= orbitSpeed;
        }
        if (params->dDown) {
            orbitAngleY += orbitSpeed;
        }

        glm::vec3 camFront = glm::normalize(glm::vec3(0) - params->camPos);
        glm::vec3 right = glm::normalize(glm::cross(camFront, glm::vec3(0,1,0)));

        // Construct the rotation matrices
        glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), glm::radians(orbitAngleX), right);
        glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(orbitAngleY), glm::vec3(0.0f, 1.0f, 0.0f));

        // Apply the rotations to the camera position
        glm::vec4 rotatedPosition = glm::vec4(params->camPos, 1.0f) * rotateX * rotateY;
        params->camPos = glm::vec3(rotatedPosition);
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

    if (glewInit() !=GLEW_OK)
    {
        std::cout << "GLEW fail! :(\n" << std::endl;
        return -3;
    }

    //START
    Params params;
    glfwSetWindowUserPointer(window, &params);

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
    phongShader.setVec3("uViewPos", 0.0, 0.0, 5.0);

    glm::mat4 projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
    phongShader.setMat4("uProjection", projectionP);


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

        //Camera
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::lookAt(params.camPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        phongShader.setMat4("uView", view);
        phongShader.setVec3("uViewPos", params.camPos);

        //Scene
        currentRot += 0.5;
        if (currentRot >= 360) {
            currentRot = 0 + 360 - currentRot;
        }
        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 4.0));
        m = glm::rotate(m, glm::radians(currentRot), glm::vec3(0.5, 1.0, 0.0));
        m = glm::scale(m, glm::vec3(0.5));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(kockaDif, kockaSpec);

        m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -2.0, 0.0));
        m = glm::scale(m, glm::vec3(12.0, 0.5, 12.0));
        phongShader.setMat4("uModel", m);
        simpleCube->Render(texture, kockaSpec);

        model2 = glm::rotate(model2, glm::radians(0.4f), glm::vec3(0.5f, 1.0f, 0.5f));
        phongShader.setMat4("uModel", model2);
        lija.Draw(phongShader);



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


