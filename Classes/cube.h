#pragma once
#include "figure.h"

class Cube : public Figure {
public:
	Cube(int division = 4, glm::vec3 start = glm::vec3(0), float length = 1.f);

	void Render(int colorLoc) override;
	void RenderWireframe(int colorLoc);
	void RenderCps(int colorLoc);

	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> GenerateData(int division, glm::vec3 center, float length);

	std::vector<glm::vec3> GetControlPoints() { return controlPoints; }
	void SetControlPoints(std::vector<glm::vec3> cps);
	std::vector<glm::vec3> GetCorners();

private:
	std::vector<glm::vec3> controlPoints;

	std::vector<glm::vec3> calculateControlPoints(glm::vec3 start, float length, int division) const;
};