#include "restrainingCube.h"
#include <glm/gtc/type_ptr.hpp>

const glm::vec4 color = glm::vec4(0, 1, 0, 1);

RestrainingCube::RestrainingCube(glm::vec3 center, float size) 
	: Figure(GenerateData(center, size)) {
	this->controlPoints = GenerateControlPoints(center, size);
}

void RestrainingCube::Render(int colorLoc)
{
	vao.Bind();

	glUniform4fv(colorLoc, 1, glm::value_ptr(color));
	glDrawElements(GL_LINES, indices_count, GL_UNSIGNED_INT, 0);

	vao.Unbind();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> RestrainingCube::GenerateData(glm::vec3 center, float size)
{
	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;

	float halfSize = size / 2.f;

    this->controlPoints = this->GenerateControlPoints(center, size);

    for (const auto& cp : controlPoints) {
		vertices.push_back(cp.x);
		vertices.push_back(cp.y);
		vertices.push_back(cp.z);
    }

    indices = {
        0, 1,   1, 2,   2, 3,   3, 0,
        4, 5,   5, 6,   6, 7,   7, 4,
        0, 4,   1, 5,   2, 6,   3, 7
    };

	return std::make_tuple(vertices, indices);
}

std::vector<glm::vec3> RestrainingCube::GenerateControlPoints(glm::vec3 center, float size)
{
    float halfSize = size / 2.f;

    return {
        center + glm::vec3(-halfSize, -halfSize, -halfSize),
        center + glm::vec3(halfSize, -halfSize, -halfSize),
        center + glm::vec3(halfSize,  halfSize, -halfSize),
        center + glm::vec3(-halfSize,  halfSize, -halfSize),
        center + glm::vec3(-halfSize, -halfSize,  halfSize),
        center + glm::vec3(halfSize, -halfSize,  halfSize),
        center + glm::vec3(halfSize,  halfSize,  halfSize),
        center + glm::vec3(-halfSize,  halfSize,  halfSize)
    };
}
