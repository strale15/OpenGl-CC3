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

	//Parking
	bool rampUp = false;

	bool spot1Taken = false;
	bool spot2Taken = false;
	bool spot3Taken = false;
	bool spot4Taken = false;
	bool spot5Taken = false;
	bool spot6Taken = false;

	float spot1Time = 0;
	float spot2Time = 0;
	float spot3Time = 0;
	float spot4Time = 0;
	float spot5Time = 0;
	float spot6Time = 0;

	glm::vec3 spot1Color = glm::vec3(0);
	glm::vec3 spot2Color = glm::vec3(0);
	glm::vec3 spot3Color = glm::vec3(0);
	glm::vec3 spot4Color = glm::vec3(0);
	glm::vec3 spot5Color = glm::vec3(0);
	glm::vec3 spot6Color = glm::vec3(0);

	int activeCamera = 0;
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

	//Parking
	float time = 20;
	//cout << params->spot1Time << endl;
	if (params->spot1Taken) {
		params->spot1Time += params->dt;
		if (params->spot1Time >= time) {
			params->spot1Taken = false;
			params->spot2Time = 0;
		}
	}
	if (params->spot2Taken) {
		params->spot2Time += params->dt;
		if (params->spot2Time >= time) {
			params->spot2Taken = false;
			params->spot2Time = 0;
		}
	}
	if (params->spot3Taken) {
		params->spot3Time += params->dt;
		if (params->spot3Time >= time) {
			params->spot3Taken = false;
			params->spot3Time = 0;
		}
	}
	if (params->spot4Taken) {
		params->spot4Time += params->dt;
		if (params->spot4Time >= time) {
			params->spot4Taken = false;
			params->spot4Time = 0;
		}
	}
	if (params->spot5Taken) {
		params->spot5Time += params->dt;
		if (params->spot5Time >= time) {
			params->spot5Taken = false;
			params->spot5Time = 0;
		}
	}
	if (params->spot6Taken) {
		params->spot6Time += params->dt;
		if (params->spot6Time >= time) {
			params->spot6Taken = false;
			params->spot6Time = 0;
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

	//Parking
	/*if (key == GLFW_KEY_E) {
		if (action == GLFW_PRESS) {
			params->rotRight = true;
		}
		else if (action == GLFW_RELEASE) {
			params->rotRight = false;
		}
	}*/

	if (key == GLFW_KEY_F) {
		if (action == GLFW_PRESS) {
			params->rampUp = !params->rampUp;
		}
	}
	if (key == GLFW_KEY_C) {
		if (action == GLFW_PRESS) {
			params->activeCamera = (params->activeCamera + 1) % 4;
		}
	}

	if (key == GLFW_KEY_1 && action == GLFW_PRESS && mode == GLFW_MOD_ALT) {
		if (!params->spot1Taken) {
			params->spot1Taken = true;
			params->spot1Time = 0;

			float randomNumber1 = static_cast<float>(std::rand()) / RAND_MAX;
			float randomNumber2 = static_cast<float>(std::rand()) / RAND_MAX;
			float randomNumber3 = static_cast<float>(std::rand()) / RAND_MAX;

			params->spot1Color = glm::vec3(randomNumber1, randomNumber2, randomNumber3);
		}
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS && mode == GLFW_MOD_ALT) {
		if (!params->spot2Taken) {
			params->spot2Taken = true;
			params->spot2Time = 0;

			float randomNumber1 = static_cast<float>(std::rand()) / RAND_MAX;
			float randomNumber2 = static_cast<float>(std::rand()) / RAND_MAX;
			float randomNumber3 = static_cast<float>(std::rand()) / RAND_MAX;

			params->spot2Color = glm::vec3(randomNumber1, randomNumber2, randomNumber3);
		}
	}

	if (key == GLFW_KEY_3 && action == GLFW_PRESS && mode == GLFW_MOD_ALT) {
		if (!params->spot3Taken) {
			params->spot3Taken = true;
			params->spot3Time = 0;

			float randomNumber1 = static_cast<float>(std::rand()) / RAND_MAX;
			float randomNumber2 = static_cast<float>(std::rand()) / RAND_MAX;
			float randomNumber3 = static_cast<float>(std::rand()) / RAND_MAX;

			params->spot3Color = glm::vec3(randomNumber1, randomNumber2, randomNumber3);
		}
	}

	if (key == GLFW_KEY_4 && action == GLFW_PRESS && mode == GLFW_MOD_ALT) {
		if (!params->spot4Taken) {
			params->spot4Taken = true;
			params->spot4Time = 0;

			float randomNumber1 = static_cast<float>(std::rand()) / RAND_MAX;
			float randomNumber2 = static_cast<float>(std::rand()) / RAND_MAX;
			float randomNumber3 = static_cast<float>(std::rand()) / RAND_MAX;

			params->spot4Color = glm::vec3(randomNumber1, randomNumber2, randomNumber3);
		}
	}

	if (key == GLFW_KEY_5 && action == GLFW_PRESS && mode == GLFW_MOD_ALT) {
		if (!params->spot5Taken) {
			params->spot5Taken = true;
			params->spot5Time = 0;

			float randomNumber1 = static_cast<float>(std::rand()) / RAND_MAX;
			float randomNumber2 = static_cast<float>(std::rand()) / RAND_MAX;
			float randomNumber3 = static_cast<float>(std::rand()) / RAND_MAX;

			params->spot5Color = glm::vec3(randomNumber1, randomNumber2, randomNumber3);
		}
	}

	if (key == GLFW_KEY_6 && action == GLFW_PRESS && mode == GLFW_MOD_ALT) {
		if (!params->spot6Taken) {
			params->spot6Taken = true;
			params->spot6Time = 0;

			float randomNumber1 = static_cast<float>(std::rand()) / RAND_MAX;
			float randomNumber2 = static_cast<float>(std::rand()) / RAND_MAX;
			float randomNumber3 = static_cast<float>(std::rand()) / RAND_MAX;

			params->spot6Color = glm::vec3(randomNumber1, randomNumber2, randomNumber3);
		}
	}

	if (key == GLFW_KEY_1 && action == GLFW_PRESS && mode == GLFW_MOD_SHIFT) {
		params->spot1Time = 0;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS && mode == GLFW_MOD_SHIFT) {
		params->spot2Time = 0;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS && mode == GLFW_MOD_SHIFT) {
		params->spot3Time = 0;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS && mode == GLFW_MOD_SHIFT) {
		params->spot4Time = 0;
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS && mode == GLFW_MOD_SHIFT) {
		params->spot5Time = 0;
	}
	if (key == GLFW_KEY_6 && action == GLFW_PRESS && mode == GLFW_MOD_SHIFT) {
		params->spot6Time = 0;
	}
}

int main()
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
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

	Model lija("res/low-poly-fox.obj");

	Shader phongShader("phong.vert", "phong.frag");
	Shader hudShader("hud.vert", "hud.frag");
	Shader twoD("twoD.vert", "twoD.frag");

	phongShader.use();

	glm::mat4 view;
	glm::mat4 projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
	phongShader.setMat4("uProjection", projectionP);

	phongShader.setVec3("uDirLight.Position", 0.0, 5, 0.0);
	phongShader.setVec3("uDirLight.Direction", 0.1, -5, 0.1);
	phongShader.setVec3("uDirLight.Ka", glm::vec3(0.2));
	phongShader.setVec3("uDirLight.Kd", glm::vec3(0.3));
	phongShader.setVec3("uDirLight.Ks", glm::vec3(1.0));

	phongShader.setVec3("uSpotlights[0].Position", glm::vec3(-999));
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

	unsigned hudTex = Model::textureFromFile("res/hudTex.png");
	unsigned kockaDif = Model::textureFromFile("res/container_diffuse.png");
	unsigned kockaSpec = Model::textureFromFile("res/container_specular.png");
	unsigned betonDif = Model::textureFromFile("res/betonDif.png");
	unsigned betonSpec = Model::textureFromFile("res/betonSpec.png");
	unsigned betonDifPod = Model::textureFromFile("res/betonDifPod.png");
	unsigned betonSpecPod = Model::textureFromFile("res/betonSpecPod.png");

	phongShader.setInt("uMaterial.Kd", 0);
	phongShader.setInt("uMaterial.Ks", 1);
	phongShader.setFloat("uMaterial.Shininess", 0.5 * 128);

	glm::mat4 model2 = glm::mat4(1.0f);
	glm::mat4 m(1.0f);
	float currentRot = 0;
	float FrameStartTime = 0;
	float FrameEndTime = 0;
	float rampRot = 0;
	float isRampUp = false;
	float yaw = 0;

	Params params;
	glfwSetWindowUserPointer(window, &params);

	glClearColor(0.2, 0.2, 0.6, 1.0);
	glEnable(GL_DEPTH_TEST);
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

		if (params.activeCamera == 0) {
			glm::vec3 front;
			yaw = params.camYaw;
			glm::vec3 camPos = glm::vec3(-11.5, 7.3, -11.5);
			float dotProduct = glm::dot(glm::normalize(camPos), glm::vec3(0));
			float angle = glm::acos(dotProduct);
			angle = glm::radians(-30.f);

			front.x = cos(glm::radians(yaw)) * cos(angle);
			front.y = sin(angle);
			front.z = sin(glm::radians(yaw)) * cos(angle);

			front = glm::normalize(front);

			view = glm::lookAt(camPos, camPos + front, glm::vec3(0, 1, 0));

			projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
			phongShader.setMat4("uProjection", projectionP);
		}
		else if (params.activeCamera == 1)
		{
			glm::vec3 front;
			yaw = params.camYaw;
			glm::vec3 camPos = glm::vec3(11.5, 7.3, 11.5);
			float dotProduct = glm::dot(glm::normalize(camPos), glm::vec3(0));
			float angle = glm::acos(dotProduct);
			angle = glm::radians(-30.f);

			front.x = cos(glm::radians(yaw)) * cos(angle);
			front.y = sin(angle);
			front.z = sin(glm::radians(yaw)) * cos(angle);

			front = glm::normalize(front);

			view = glm::lookAt(camPos, camPos + front, glm::vec3(0, 1, 0));

			projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
			phongShader.setMat4("uProjection", projectionP);
		}
		else if (params.activeCamera == 2)
		{
			glm::vec3 front;
			yaw = -90;
			glm::vec3 camPos = glm::vec3(10, 3.1, 11.7);
			float dotProduct = glm::dot(glm::normalize(camPos), glm::vec3(0));
			float angle = glm::acos(dotProduct);
			angle = glm::radians(-30.f);

			front.x = cos(glm::radians(yaw)) * cos(angle);
			front.y = sin(angle);
			front.z = sin(glm::radians(yaw)) * cos(angle);

			front = glm::normalize(front);

			view = glm::lookAt(camPos, camPos + front, glm::vec3(0, 1, 0));

			projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
			phongShader.setMat4("uProjection", projectionP);
		}
		else if (params.activeCamera == 3)
		{
			glm::vec3 camPos = glm::vec3(0, 20, 0);
			view = glm::lookAt(camPos, glm::vec3(0, 0, 0), glm::vec3(1, 0, 0));

			float k = 5;
			projectionP = glm::ortho(-16.f-k, 16.f+k, -9.f-k, 9.f+k, 0.1f, 201.f);
			phongShader.setMat4("uProjection", projectionP);
		}
		else {
			view = glm::lookAt(params.position, params.position + params.cameraFront, params.cameraUp);
		}


		phongShader.setMat4("uView", view);
		phongShader.setVec3("uViewPos", params.position);

		//SCENE
		//------------------------------------------------------------------------------------------------------------
		m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
		m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
		m = glm::scale(m, glm::vec3(1.0, 1.0, 1.0));
		phongShader.setMat4("uModel", m);
		//simpleCube->Render(&phongShader, 0, 1, 1);

		//Pod
		m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
		m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
		m = glm::scale(m, glm::vec3(-25.0, 1.0, 25.0));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, betonDifPod, betonSpecPod);

		//Zidovi
		m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -4.5, -12.5));
		m = glm::scale(m, glm::vec3(25.0, 25.0, 1.0));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, betonDif, betonSpec);

		m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -4.5, 12.5));
		m = glm::scale(m, glm::vec3(25.0, 25.0, 1.0));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, betonDif, betonSpec);

		m = glm::translate(glm::mat4(1.0), glm::vec3(-12.5, -4.5, 0.0));
		m = glm::scale(m, glm::vec3(1.0, 25.0, 25.0));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, betonDif, betonSpec);

		//Plafon
		if (params.activeCamera != 3) {
			m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 8.0, 0.0));
			m = glm::scale(m, glm::vec3(25.0, 1.0, 25.0));
			phongShader.setMat4("uModel", m);
			simpleCube->Render(&phongShader, betonDif, betonSpec);
		}

		//PortirStanica
		float xOffset = 10;

		m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 2.0, 12.0));
		m = glm::translate(m, glm::vec3(xOffset, 0, 0));
		m = glm::scale(m, glm::vec3(2.0, 4.0, 0.3));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, 1, 0, 0);

		m = glm::translate(glm::mat4(1.0), glm::vec3(1.0, 2.0, 11.0));
		m = glm::translate(m, glm::vec3(xOffset, 0, 0));
		m = glm::scale(m, glm::vec3(0.3, 4.0, 2.0));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, 1, 0, 0);

		m = glm::translate(glm::mat4(1.0), glm::vec3(-1.0, 2.0, 11.0));
		m = glm::translate(m, glm::vec3(xOffset, 0, 0));
		m = glm::scale(m, glm::vec3(0.3, 4.0, 2.0));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, 1, 0, 0);

		m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 1.0, 10.0));
		m = glm::translate(m, glm::vec3(xOffset, 0, 0));
		m = glm::scale(m, glm::vec3(2.0, 2.0, 0.3));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, 1, 0, 0);

		m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 4.0, 11.0));
		m = glm::translate(m, glm::vec3(xOffset, 0, 0));
		m = glm::scale(m, glm::vec3(2.0, 0.1, 2.0));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, 1, 0, 0);

		phongShader.setVec3("uPointLights[0].Position", glm::vec3(xOffset, 3.7, 11.0));
		phongShader.setVec3("uPointLights[0].Ka", glm::vec3(0.6, 0.6, 0.2) / 2.f);
		phongShader.setVec3("uPointLights[0].Kd", glm::vec3(0.6, 0.6, 0.2));
		phongShader.setVec3("uPointLights[0].Ks", glm::vec3(0.6, 0.6, 0.2));
		phongShader.setFloat("uPointLights[0].Kc", 1.5f);
		phongShader.setFloat("uPointLights[0].Kl", 1.0f);
		phongShader.setFloat("uPointLights[0].Kq", 0.272f);

		//Display
		m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 2.0, 10.2));
		m = glm::translate(m, glm::vec3(xOffset, 0, 0));
		m = glm::scale(m, glm::vec3(0.7, 0.7, 0.1));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, 0, 0, 1);

		//Test cars
		//Left
		if (params.spot3Taken) {
			m = glm::translate(glm::mat4(1.0), glm::vec3(3.125, 1.0, -8.5));
			m = glm::scale(m, glm::vec3(1.0, 1.0, 1.0));
			phongShader.setMat4("uModel", m);
			simpleCube->Render(&phongShader, params.spot3Color.x, params.spot3Color.y, params.spot3Color.z);
		}

		if (params.spot4Taken) {
			m = glm::translate(glm::mat4(1.0), glm::vec3(3.125 * 3, 1.0, -8.5));
			m = glm::scale(m, glm::vec3(1.0, 1.0, 1.0));
			phongShader.setMat4("uModel", m);
			simpleCube->Render(&phongShader, params.spot4Color.x, params.spot4Color.y, params.spot4Color.z);
		}

		if (params.spot2Taken) {
			m = glm::translate(glm::mat4(1.0), glm::vec3(-3.125, 1.0, -8.5));
			m = glm::scale(m, glm::vec3(1.0, 1.0, 1.0));
			phongShader.setMat4("uModel", m);
			simpleCube->Render(&phongShader, params.spot2Color.x, params.spot2Color.y, params.spot2Color.z);
		}

		if (params.spot1Taken) {
			m = glm::translate(glm::mat4(1.0), glm::vec3(-3.125 * 3, 1.0, -8.5));
			m = glm::scale(m, glm::vec3(1.0, 1.0, 1.0));
			phongShader.setMat4("uModel", m);
			simpleCube->Render(&phongShader, params.spot1Color.x, params.spot1Color.y, params.spot1Color.z);
		}

		//Right
		if (params.spot6Taken) {
			m = glm::translate(glm::mat4(1.0), glm::vec3(-3.125, 1.0, 8.5));
			m = glm::scale(m, glm::vec3(1.0, 1.0, 1.0));
			phongShader.setMat4("uModel", m);
			simpleCube->Render(&phongShader, params.spot6Color.x, params.spot6Color.y, params.spot6Color.z);
		}

		if (params.spot5Taken) {
			m = glm::translate(glm::mat4(1.0), glm::vec3(-3.125 * 3, 1.0, 8.5));
			m = glm::scale(m, glm::vec3(1.0, 1.0, 1.0));
			phongShader.setMat4("uModel", m);
			simpleCube->Render(&phongShader, params.spot5Color.x, params.spot5Color.y, params.spot5Color.z);
		}

		//Ramp
		if (params.rampUp || isRampUp && rampRot < 90) {
			isRampUp = true;
			rampRot += 20 * params.dt;
		}
		else if (!params.rampUp || !isRampUp && rampRot > 0) {
			isRampUp = false;
			rampRot -= 20 * params.dt;
		}

		rampRot = glm::clamp(rampRot, 0.f, 90.f);

		m = glm::translate(glm::mat4(1.0), glm::vec3(12.0, 1.5, 4.5));
		m = glm::rotate(m, glm::radians(rampRot), glm::vec3(1.0, 0.0, 0.0));
		m = glm::translate(m, -glm::vec3(12.0, 1.5, 4.5));

		m = glm::translate(m, glm::vec3(12.0, 1.5, 0.0));
		m = glm::scale(m, glm::vec3(0.2, 0.2, 9.0));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, 1, 0, 1);

		//RampHold
		m = glm::translate(glm::mat4(1.0), glm::vec3(12.0, 1.0, 4.5));
		m = glm::scale(m, glm::vec3(1.0, 2.0, 1.0));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, 1, 0, 1);

		//Glass
		m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 3.0, 10 - 0.07));
		m = glm::translate(m, glm::vec3(xOffset, 0, 0));
		m = glm::scale(m, glm::vec3(2.0, 2.0, 0.15));
		phongShader.setBool("uTransp", true);
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, 1, 0, 0);
		phongShader.setBool("uTransp", false);






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