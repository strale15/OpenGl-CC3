#include <iostream>
#include <vector>
#include <GL/glew.h>
#include "shader.hpp"

class GameObject {
private:
	unsigned int VAO, VBO, EBO;
	unsigned int vCount;
public:
	static int rCount;
	GameObject() {
	}

	GameObject(std::vector<float> vertices) {
		vCount = vertices.size() / 8;

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	~GameObject() {

		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
	}

	void Render(Shader* shader, unsigned const& texture, unsigned const& specularMap) {
		shader->setBool("isColor", false);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		if (specularMap != NULL) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, specularMap);
		}

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vCount);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}

	void Render(Shader* shader, unsigned const& texture) {
		shader->setBool("isColor", false);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vCount);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}

	void Render(Shader* shader, float r, float g, float b) {
		shader->setBool("isColor", true);
		shader->setVec3("uColor", glm::vec3(r, g, b));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vCount);

		glBindVertexArray(0);
		shader->setBool("isColor", false);
	}

	void Render() {
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vCount);
		glBindVertexArray(0);
	}
};