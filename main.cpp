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

	float spot1Time = 20;
	float spot2Time = 20;
	float spot3Time = 20;
	float spot4Time = 20;
	float spot5Time = 20;
	float spot6Time = 20;

	glm::vec3 spot1Color = glm::vec3(0);
	glm::vec3 spot2Color = glm::vec3(0);
	glm::vec3 spot3Color = glm::vec3(0);
	glm::vec3 spot4Color = glm::vec3(0);
	glm::vec3 spot5Color = glm::vec3(0);
	glm::vec3 spot6Color = glm::vec3(0);

	int activeCamera = 0;

	bool isTransp = false;

	bool isWireFrame = false;
	bool isLit = true;
};

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
			params->spot2Time = 20;
		}
	}
	if (params->spot2Taken) {
		params->spot2Time += params->dt;
		if (params->spot2Time >= time) {
			params->spot2Taken = false;
			params->spot2Time = 20;
		}
	}
	if (params->spot3Taken) {
		params->spot3Time += params->dt;
		if (params->spot3Time >= time) {
			params->spot3Taken = false;
			params->spot3Time = 20;
		}
	}
	if (params->spot4Taken) {
		params->spot4Time += params->dt;
		if (params->spot4Time >= time) {
			params->spot4Taken = false;
			params->spot4Time = 20;
		}
	}
	if (params->spot5Taken) {
		params->spot5Time += params->dt;
		if (params->spot5Time >= time) {
			params->spot5Taken = false;
			params->spot5Time = 20;
		}
	}
	if (params->spot6Taken) {
		params->spot6Time += params->dt;
		if (params->spot6Time >= time) {
			params->spot6Taken = false;
			params->spot6Time = 20;
		}
	}

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
	if (key == GLFW_KEY_B) {
		if (action == GLFW_PRESS) {
			params->isTransp = !params->isTransp;
		}
	}
	if (key == GLFW_KEY_K) {
		if (action == GLFW_PRESS) {
			params->isWireFrame = !params->isWireFrame;
		}
	}
	if (key == GLFW_KEY_L) {
		if (action == GLFW_PRESS) {
			params->isLit = !params->isLit;
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
		if(params->spot1Taken)
			params->spot1Time = 0;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS && mode == GLFW_MOD_SHIFT) {
		if (params->spot2Taken)
		params->spot2Time = 0;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS && mode == GLFW_MOD_SHIFT) {
		if (params->spot3Taken)
		params->spot3Time = 0;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS && mode == GLFW_MOD_SHIFT) {
		if (params->spot4Taken)
		params->spot4Time = 0;
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS && mode == GLFW_MOD_SHIFT) {
		if (params->spot5Taken)
		params->spot5Time = 0;
	}
	if (key == GLFW_KEY_6 && action == GLFW_PRESS && mode == GLFW_MOD_SHIFT) {
		if (params->spot6Taken)
		params->spot6Time = 0;
	}

	if (key == GLFW_KEY_1 && action == GLFW_PRESS && mode == GLFW_MOD_CONTROL) {
		params->spot1Time = 20;
		params->spot1Taken = false;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS && mode == GLFW_MOD_CONTROL) {
		params->spot2Time = 20;
		params->spot2Taken = false;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS && mode == GLFW_MOD_CONTROL) {
		params->spot3Time = 20;
		params->spot3Taken = false;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS && mode == GLFW_MOD_CONTROL) {
		params->spot4Time = 20;
		params->spot4Taken = false;
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS && mode == GLFW_MOD_CONTROL) {
		params->spot5Time = 20;
		params->spot5Taken = false;
	}
	if (key == GLFW_KEY_6 && action == GLFW_PRESS && mode == GLFW_MOD_CONTROL) {
		params->spot6Time = 20;
		params->spot6Taken = false;
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

	Model carModel("res/1377 Car.obj");

	Shader phongShader("phong.vert", "phong.frag");
	Shader twoD("twoD.vert", "twoD.frag");

	phongShader.use();

	glm::mat4 view;
	glm::mat4 projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
	phongShader.setMat4("uProjection", projectionP);

	phongShader.setVec3("uDirLight.Position", 0.0, 5, 0.0);
	phongShader.setVec3("uDirLight.Direction", 0.1, -5, 0.1);
	phongShader.setVec3("uDirLight.Ka", glm::vec3(0.0));
	phongShader.setVec3("uDirLight.Kd", glm::vec3(0.0));
	phongShader.setVec3("uDirLight.Ks", glm::vec3(0.0));

	unsigned hudTex = Model::textureFromFile("res/hudTex.png");
	unsigned kockaDif = Model::textureFromFile("res/container_diffuse.png");
	unsigned kockaSpec = Model::textureFromFile("res/container_specular.png");
	unsigned betonDif = Model::textureFromFile("res/betonDif.png");
	unsigned betonSpec = Model::textureFromFile("res/betonSpec.png");
	unsigned betonDifPod = Model::textureFromFile("res/betonDifPod.png");
	unsigned betonSpecPod = Model::textureFromFile("res/betonSpecPod.png");
	unsigned betonEmm = Model::textureFromFile("res/betonEmm.png");

	phongShader.setInt("uMaterial.Kd", 0);
	phongShader.setInt("uMaterial.Ks", 1);
	phongShader.setInt("uMaterial.Ke", 2);
	phongShader.setFloat("uMaterial.Shininess", 0.5 * 128);

	glm::mat4 model2 = glm::mat4(1.0f);
	glm::mat4 m(1.0f);
	float currentRot = 0;
	float FrameStartTime = 0;
	float FrameEndTime = 0;
	float rampRot = 0;
	float isRampUp = false;
	float yaw1 = 0;
	float yaw2 = 0;
	bool cam1Rot = false;
	bool cam2Rot = false;

	Params params;
	glfwSetWindowUserPointer(window, &params);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	while (!glfwWindowShouldClose(window))
	{
		FrameStartTime = glfwGetTime();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		phongShader.setBool("isLit", params.isLit);

		//Loop
		phongShader.use();
		HandleInput(&params);

		//Camera
		if (yaw1 >= 90) {
			cam1Rot = false;
		}
		else if(yaw1 <= 0)
		{
			cam1Rot = true;
		}

		if (!cam1Rot) {
			yaw1 -= 20*params.dt;
		}
		else
		{
			yaw1 += 20*params.dt;
		}

		//Cam 1
		glm::vec3 front1;
		glm::vec3 camPos1 = glm::vec3(-11.5, 7.3, -11.5);
		float angle1 = glm::radians(-30.f);

		front1.x = cos(glm::radians(yaw1)) * cos(angle1);
		front1.y = sin(angle1);
		front1.z = sin(glm::radians(yaw1)) * cos(angle1);

		front1 = glm::normalize(front1);

		//Cam 2
		glm::vec3 front2;
		glm::vec3 camPos2 = glm::vec3(11.5, 7.3, 11.5);
		float angle2 = glm::radians(-30.f);

		front2.x = cos(glm::radians(-180+yaw1)) * cos(angle2);
		front2.y = sin(angle2);
		front2.z = sin(glm::radians(-180+yaw1)) * cos(angle2);

		front2 = glm::normalize(front2);

		//Spotlights
		phongShader.setVec3("uSpotlights[0].Position", camPos1);
		phongShader.setVec3("uSpotlights[0].Direction", front1);
		phongShader.setVec3("uSpotlights[0].Ka", 0.0, 0.0, 0.0);
		phongShader.setVec3("uSpotlights[0].Kd", glm::vec3(3));
		phongShader.setVec3("uSpotlights[0].Ks", glm::vec3(10.0));
		phongShader.setFloat("uSpotlights[0].InnerCutOff", glm::cos(glm::radians(15.0f)));
		phongShader.setFloat("uSpotlights[0].OuterCutOff", glm::cos(glm::radians(25.0f)));
		phongShader.setFloat("uSpotlights[0].Kc", 1.0);
		phongShader.setFloat("uSpotlights[0].Kl", 0.092f);
		phongShader.setFloat("uSpotlights[0].Kq", 0.0032f);

		phongShader.setVec3("uSpotlights[1].Position", camPos2);
		phongShader.setVec3("uSpotlights[1].Direction", front2);
		phongShader.setVec3("uSpotlights[1].Ka", 0.0, 0.0, 0.0);
		phongShader.setVec3("uSpotlights[1].Kd", glm::vec3(3));
		phongShader.setVec3("uSpotlights[1].Ks", glm::vec3(10));
		phongShader.setFloat("uSpotlights[1].InnerCutOff", glm::cos(glm::radians(15.0f)));
		phongShader.setFloat("uSpotlights[1].OuterCutOff", glm::cos(glm::radians(25.0f)));
		phongShader.setFloat("uSpotlights[1].Kc", 1.0);
		phongShader.setFloat("uSpotlights[1].Kl", 0.092f);
		phongShader.setFloat("uSpotlights[1].Kq", 0.0032f);

		if (params.activeCamera == 0) {
			view = glm::lookAt(camPos1, camPos1 + front1, glm::vec3(0, 1, 0));

			projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
			phongShader.setMat4("uProjection", projectionP);
			phongShader.setVec3("uViewPos", camPos1);
		}
		else if (params.activeCamera == 1)
		{
			view = glm::lookAt(camPos2, camPos2 + front2, glm::vec3(0, 1, 0));

			projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
			phongShader.setMat4("uProjection", projectionP);
			phongShader.setVec3("uViewPos", camPos2);
		}
		else if (params.activeCamera == 2)
		{
			glm::vec3 front;
			float yaw = -90;
			glm::vec3 camPos = glm::vec3(10, 3.1, 11.7);
			float angle = glm::radians(-30.f);

			front.x = cos(glm::radians(yaw)) * cos(angle);
			front.y = sin(angle);
			front.z = sin(glm::radians(yaw)) * cos(angle);

			front = glm::normalize(front);

			view = glm::lookAt(camPos, camPos + front, glm::vec3(0, 1, 0));

			projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
			phongShader.setMat4("uProjection", projectionP);
			phongShader.setVec3("uViewPos", camPos);
		}
		else if (params.activeCamera == 3)
		{
			glm::vec3 camPos = glm::vec3(0, 20, 0);
			view = glm::lookAt(camPos, glm::vec3(0, 0, 0), glm::vec3(1, 0, 0));

			float k = 5;
			projectionP = glm::ortho(-16.f-k, 16.f+k, -9.f-k, 9.f+k, 0.1f, 201.f);
			phongShader.setMat4("uProjection", projectionP);
			phongShader.setVec3("uViewPos", camPos);
		}

		//view = glm::lookAt(params.position, params.position + params.cameraFront, params.cameraUp);

		twoD.use();
		twoD.setMat4("uProjection", projectionP);
		twoD.setMat4("uView", view);
		phongShader.use();


		phongShader.setMat4("uView", view);

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
		m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 0.0, 1.0));
		m = glm::scale(m, glm::vec3(25.0, 25.0, -1.0));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, betonDif, betonSpec, betonEmm, true);

		m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -4.5, 12.5));
		m = glm::rotate(m, glm::radians(180.f), glm::vec3(1.0, 0.0, 0.0));
		m = glm::scale(m, glm::vec3(25.0, 25.0, -1.0));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, betonDif, betonSpec);

		m = glm::translate(glm::mat4(1.0), glm::vec3(-12.5, -4.5, 0.0));
		m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 0.0, 1.0));
		m = glm::scale(m, glm::vec3(-1.0, 25.0, 25.0));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, betonDif, betonSpec, betonEmm, true);

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
		phongShader.setVec3("uPointLights[0].Kd", glm::vec3(0.6, 0.6, 0.2)*2.f);
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
			m = glm::translate(glm::mat4(1.0), glm::vec3(3.125, 0.5, -8.5));
			m = glm::scale(m, glm::vec3(0.07));
			phongShader.setMat4("uModel", m);
			phongShader.setBool("isColor", true);
			phongShader.setVec3("uColor", params.spot3Color);
			carModel.Draw(phongShader);
			phongShader.setBool("isColor", false);
		}

		if (params.spot4Taken) {
			m = glm::translate(glm::mat4(1.0), glm::vec3(3.125 * 3, 0.5, -8.5));
			m = glm::scale(m, glm::vec3(0.07));
			phongShader.setMat4("uModel", m);
			phongShader.setBool("isColor", true);
			phongShader.setVec3("uColor", params.spot4Color);
			carModel.Draw(phongShader);
			phongShader.setBool("isColor", false);
		}

		if (params.spot2Taken) {
			m = glm::translate(glm::mat4(1.0), glm::vec3(-3.125, 0.5, -8.5));
			m = glm::scale(m, glm::vec3(0.07));
			phongShader.setMat4("uModel", m);
			phongShader.setBool("isColor", true);
			phongShader.setVec3("uColor", params.spot2Color);
			carModel.Draw(phongShader);
			phongShader.setBool("isColor", false);
		}

		if (params.spot1Taken) {
			m = glm::translate(glm::mat4(1.0), glm::vec3(-3.125 * 3, 0.5, -8.5));
			m = glm::scale(m, glm::vec3(0.07));
			phongShader.setMat4("uModel", m);
			phongShader.setBool("isColor", true);
			phongShader.setVec3("uColor", params.spot1Color);
			carModel.Draw(phongShader);
			phongShader.setBool("isColor", false);
		}

		//Right
		if (params.spot6Taken) {
			m = glm::translate(glm::mat4(1.0), glm::vec3(-3.125, 0.5, 8.5));
			m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
			m = glm::scale(m, glm::vec3(0.07));
			phongShader.setMat4("uModel", m);
			phongShader.setBool("isColor", true);
			phongShader.setVec3("uColor", params.spot6Color);
			carModel.Draw(phongShader);
			phongShader.setBool("isColor", false);
		}

		if (params.spot5Taken) {
			m = glm::translate(glm::mat4(1.0), glm::vec3(-3.125 * 3, 0.5, 8.5));
			m = glm::rotate(m, glm::radians(180.f), glm::vec3(0.0, 1.0, 0.0));
			m = glm::scale(m, glm::vec3(0.07));
			phongShader.setMat4("uModel", m);
			phongShader.setBool("isColor", true);
			phongShader.setVec3("uColor", params.spot5Color);
			carModel.Draw(phongShader);
			phongShader.setBool("isColor", false);
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
		simpleCube->Render(&phongShader, 1, 0, 0);

		//RampHold
		m = glm::translate(glm::mat4(1.0), glm::vec3(12.0, 1.0, 4.5));
		m = glm::scale(m, glm::vec3(1.0, 2.0, 1.0));
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, 1, 1, 1);

		//Glass
		m = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 3.0, 10 - 0.07));
		m = glm::translate(m, glm::vec3(xOffset, 0, 0));
		m = glm::scale(m, glm::vec3(2.0, 2.0, 0.15));
		phongShader.setBool("uTransp", true);
		phongShader.setMat4("uModel", m);
		simpleCube->Render(&phongShader, 0.43, 0.45, 0.45);
		phongShader.setBool("uTransp", false);


		//2D Display
		twoD.use();
		//glm::vec3(10.0, 2.0, 10.251)

		m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0, 2.0, 10.251));
		m = glm::scale(m, glm::vec3(0.66));
		twoD.setMat4("uModel", m);
		rectangle->Render(&twoD, betonDifPod);

		//Taken ind
		float offset = 0.165/2;

		//spot1
		float spotScale = 0.1f + ((params.spot1Time) / 20.0f) * (0.0f - 0.1f);
		float spotOffset = (0.1 - spotScale) / 2;

		m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 - offset * 3, 2.0 + 0.31, 10.252));
		m = glm::scale(m, glm::vec3(0.03));
		twoD.setMat4("uModel", m);
		if(params.spot1Taken)
			rectangle->Render(&twoD, 1,0,0);
		else
			rectangle->Render(&twoD, 0, 1, 0);

		m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 - offset * 3 - spotOffset, 2.0 + 0.1, 10.252));
		m = glm::scale(m, glm::vec3(spotScale,0.03,1.0));
		twoD.setMat4("uModel", m);
		rectangle->Render(&twoD, 1, 1, 0);

		//spot2
		spotScale = 0.1f + ((params.spot2Time) / 20.0f) * (0.0f - 0.1f);
		spotOffset = (0.1 - spotScale) / 2;

		m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 - offset, 2.0 + 0.31, 10.252));
		m = glm::scale(m, glm::vec3(0.03));
		twoD.setMat4("uModel", m);
		if (params.spot2Taken)
			rectangle->Render(&twoD, 1, 0, 0);
		else
			rectangle->Render(&twoD, 0, 1, 0);

		m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 - offset - spotOffset, 2.0 + 0.1, 10.252));
		m = glm::scale(m, glm::vec3(spotScale, 0.03, 1.0));
		twoD.setMat4("uModel", m);
		rectangle->Render(&twoD, 1, 1, 0);

		//spot3
		spotScale = 0.1f + ((params.spot3Time) / 20.0f) * (0.0f - 0.1f);
		spotOffset = (0.1 - spotScale) / 2;

		m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 + offset, 2.0 + 0.31, 10.252));
		m = glm::scale(m, glm::vec3(0.03));
		twoD.setMat4("uModel", m);
		if (params.spot3Taken)
			rectangle->Render(&twoD, 1, 0, 0);
		else
			rectangle->Render(&twoD, 0, 1, 0);

		m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 + offset - spotOffset, 2.0 + 0.1, 10.252));
		m = glm::scale(m, glm::vec3(spotScale, 0.03, 1.0));
		twoD.setMat4("uModel", m);
		rectangle->Render(&twoD, 1, 1, 0);

		//spot4
		spotScale = 0.1f + ((params.spot4Time) / 20.0f) * (0.0f - 0.1f);
		spotOffset = (0.1 - spotScale) / 2;

		m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 + offset * 3, 2.0 + 0.31, 10.252));
		m = glm::scale(m, glm::vec3(0.03));
		twoD.setMat4("uModel", m);
		if (params.spot4Taken)
			rectangle->Render(&twoD, 1, 0, 0);
		else
			rectangle->Render(&twoD, 0, 1, 0);

		m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 + offset * 3 - spotOffset, 2.0 + 0.1, 10.252));
		m = glm::scale(m, glm::vec3(spotScale, 0.03, 1.0));
		twoD.setMat4("uModel", m);
		rectangle->Render(&twoD, 1, 1, 0);

		//spot5
		spotScale = 0.1f + ((params.spot5Time) / 20.0f) * (0.0f - 0.1f);
		spotOffset = (0.1 - spotScale) / 2;

		m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 - offset * 3, 2.0 - 0.31, 10.252));
		m = glm::scale(m, glm::vec3(0.03));
		twoD.setMat4("uModel", m);
		if (params.spot5Taken)
			rectangle->Render(&twoD, 1, 0, 0);
		else
			rectangle->Render(&twoD, 0, 1, 0);

		m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 - offset * 3 - spotOffset, 2.0 - 0.1, 10.252));
		m = glm::scale(m, glm::vec3(spotScale, 0.03, 1.0));
		twoD.setMat4("uModel", m);
		rectangle->Render(&twoD, 1, 1, 0);

		//spot6
		spotScale = 0.1f + ((params.spot6Time) / 20.0f) * (0.0f - 0.1f);
		spotOffset = (0.1 - spotScale) / 2;

		m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 - offset, 2.0 - 0.31, 10.252));
		m = glm::scale(m, glm::vec3(0.03));
		twoD.setMat4("uModel", m);
		if (params.spot6Taken)
			rectangle->Render(&twoD, 1, 0, 0);
		else
			rectangle->Render(&twoD, 0, 1, 0);

		m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 - offset - spotOffset, 2.0 - 0.1, 10.252));
		m = glm::scale(m, glm::vec3(spotScale, 0.03, 1.0));
		twoD.setMat4("uModel", m);
		rectangle->Render(&twoD, 1, 1, 0);

		//Autici
		if (params.isTransp) {
			twoD.setBool("isTransp", true);
		}
		else
		{
			twoD.setBool("isTransp", false);
		}

		if (params.spot6Taken) {
			m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 - offset, 2.0 - 0.2, 10.252));
			m = glm::scale(m, glm::vec3(0.06, 0.13, 1.0));
			twoD.setMat4("uModel", m);
			rectangle->Render(&twoD, params.spot6Color.x, params.spot6Color.y, params.spot6Color.z);
		}
		if (params.spot5Taken) {
			m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 - offset*3, 2.0 - 0.2, 10.252));
			m = glm::scale(m, glm::vec3(0.06, 0.13, 1.0));
			twoD.setMat4("uModel", m);
			rectangle->Render(&twoD, params.spot5Color.x, params.spot5Color.y, params.spot5Color.z);
		}
		if (params.spot4Taken) {
			m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 + offset*3, 2.0 + 0.2, 10.252));
			m = glm::scale(m, glm::vec3(0.06, 0.13, 1.0));
			twoD.setMat4("uModel", m);
			rectangle->Render(&twoD, params.spot4Color.x, params.spot4Color.y, params.spot4Color.z);
		}
		if (params.spot3Taken) {
			m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 + offset, 2.0 + 0.2, 10.252));
			m = glm::scale(m, glm::vec3(0.06, 0.13, 1.0));
			twoD.setMat4("uModel", m);
			rectangle->Render(&twoD, params.spot3Color.x, params.spot3Color.y, params.spot3Color.z);
		}
		if (params.spot2Taken) {
			m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 - offset, 2.0 + 0.2, 10.252));
			m = glm::scale(m, glm::vec3(0.06, 0.13, 1.0));
			twoD.setMat4("uModel", m);
			rectangle->Render(&twoD, params.spot2Color.x, params.spot2Color.y, params.spot2Color.z);
		}
		if (params.spot1Taken) {
			m = glm::translate(glm::mat4(1.0f), glm::vec3(10.0 - offset*3, 2.0 + 0.2, 10.252));
			m = glm::scale(m, glm::vec3(0.06, 0.13, 1.0));
			twoD.setMat4("uModel", m);
			rectangle->Render(&twoD, params.spot1Color.x, params.spot1Color.y, params.spot1Color.z);
		}

		twoD.setBool("isTransp", false);
		phongShader.use();


		//------------------------------------------------------------------------------------------------------------

		//HUD
		//DrawHud(hudShader, hudTex);
		phongShader.use();

		glfwSwapBuffers(window);
		glfwPollEvents();

		FrameEndTime = glfwGetTime();
		params.dt = FrameEndTime - FrameStartTime;
	}

	glfwTerminate();
	return 0;
}