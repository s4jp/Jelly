#pragma once
#include "figure.h"

class RestrainingCube : public Figure {
public:
	RestrainingCube(glm::vec3 center = glm::vec3(0.5f), float size = 3.0f);

	void Render(int colorLoc) override;

	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> GenerateData(glm::vec3 center, float size);
	
	std::vector<glm::vec3> GetControlPoints() { return controlPoints; }
private:
	std::vector<glm::vec3> controlPoints;

	std::vector<glm::vec3> GenerateControlPoints(glm::vec3 center, float size);
};