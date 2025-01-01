#pragma once
#include "figure.h"

class Cube : public Figure {
public:
	std::vector<glm::vec3> controlPoints;

	Cube(int division = 4, glm::vec3 start = glm::vec3(0), float length = 1.f);

	void Render(int colorLoc) override;
	void RenderCps(int colorLoc);

	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> GenerateData(int division, glm::vec3 center, float length);
};