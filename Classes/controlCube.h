#pragma once

#include "figure.h"

class ControlCube : public Figure {
public:
	float transation[3] = { 0,0,0 };
	float rotation[3] = { 0,0,0 };

	ControlCube(std::vector<glm::vec3> startCps);

	void Render(int colorLoc) override;
	void RenderLinks(int colorLoc);

	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> InitializeAndGenerateData(std::vector<glm::vec3> startCps);
	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> GenerateData();

	void UpdateControlPoints(std::vector<glm::vec3> links);

	std::vector<glm::vec3> CalculateControlPoints();

private:
	std::vector<glm::vec3> ownControlPoints;
};