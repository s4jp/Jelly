#include "controlCube.h"
#include <glm/gtc/type_ptr.hpp>

const glm::vec4 color = glm::vec4(1, 0, 0, 1);

ControlCube::ControlCube(std::vector<glm::vec3> startCps) 
	: Figure(InitializeAndGenerateData(startCps)) {
	this->ownControlPoints = startCps;
}

void ControlCube::Render(int colorLoc)
{
	vao.Bind();

	glUniform4fv(colorLoc, 1, glm::value_ptr(color));
	glDrawElements(GL_LINES, indices_count * 3 / 5, GL_UNSIGNED_INT, 0);

	vao.Unbind();
}

void ControlCube::RenderLinks(int colorLoc)
{
	vao.Bind();

	glUniform4fv(colorLoc, 1, glm::value_ptr(color));
	glDrawElements(GL_LINES, indices_count * 2 / 5, GL_UNSIGNED_INT, (void*)(indices_count * 3 / 5 * sizeof(GLuint)));

	vao.Unbind();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> ControlCube::InitializeAndGenerateData(std::vector<glm::vec3> startCps)
{
	this->ownControlPoints = startCps;
	return GenerateData();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> ControlCube::GenerateData()
{
	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;

	std::vector<glm::vec3> cps = CalculateControlPoints();
	// for own cps
	for (auto cp : cps) {
		vertices.push_back(cp.x);
		vertices.push_back(cp.y);
		vertices.push_back(cp.z);
	}
	// for links
	for (auto cp : cps) {
		vertices.push_back(cp.x);
		vertices.push_back(cp.y);
		vertices.push_back(cp.z);
	}

	indices = {
		0, 1,	1, 2,	2, 3,	3, 0,
		4, 5,	5, 6,	6, 7,	7, 4,
		0, 4,	1, 5,	2, 6,	3, 7,
		0, 8,	1, 9,	2, 10,	3, 11,
		4, 12,	5, 13,	6, 14,	7, 15,
	};

	return std::make_tuple(vertices, indices);
}

void ControlCube::UpdateControlPoints(std::vector<glm::vec3> links)
{
	std::vector<GLfloat> vertices;
	std::vector<glm::vec3> cps = CalculateControlPoints();
	for (auto cp : cps) {
		vertices.push_back(cp.x);
		vertices.push_back(cp.y);
		vertices.push_back(cp.z);
	}
	for (auto cp : links) {
		vertices.push_back(cp.x);
		vertices.push_back(cp.y);
		vertices.push_back(cp.z);
	}
	RefreshVertices(vertices);
}

std::vector<glm::vec3> ControlCube::CalculateControlPoints()
{
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(transation[0], transation[1], transation[2]));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation[0]), glm::vec3(1.f, 0.f, 0.f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation[1]), glm::vec3(0.f, 1.f, 0.f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation[2]), glm::vec3(0.f, 0.f, 1.f));

	std::vector<glm::vec3> result;
	for (auto cp : ownControlPoints) {
		glm::vec4 transformed = modelMatrix * glm::vec4(cp, 1);
		result.push_back(glm::vec3(transformed));
	}

	return result;
}
