#include "cube.h"
#include <glm/gtc/type_ptr.hpp>

const glm::vec4 color = glm::vec4(1, 1, 1, 1);
const float cpSize = 5.f;
const int BEZIER_INDICES = 16 * 6;
const int PATCH_SIZE = 16;

Cube::Cube(int division, glm::vec3 center, float length) 
	: Figure(GenerateData(division, center, length)) {
	// work-around to avoid calling virtual function in constructor
	controlPoints = calculateControlPoints(center, length, division);
}

void Cube::Render(int colorLoc)
{
	vao.Bind();

	glUniform4fv(colorLoc, 1, glm::value_ptr(color));
	glPatchParameteri(GL_PATCH_VERTICES, PATCH_SIZE);
	glDrawElements(GL_PATCHES, BEZIER_INDICES, GL_UNSIGNED_INT, (void*)((indices_count - BEZIER_INDICES) * sizeof(GLuint)));

	vao.Unbind();
}

void Cube::RenderWireframe(int colorLoc)
{
	vao.Bind();

	glUniform4fv(colorLoc, 1, glm::value_ptr(color));
	glDrawElements(GL_LINES, indices_count - BEZIER_INDICES, GL_UNSIGNED_INT, 0);

	vao.Unbind();
}

void Cube::RenderCps(int colorLoc)
{
	vao.Bind();

	glUniform4fv(colorLoc, 1, glm::value_ptr(color));
	glPointSize(cpSize);
	glDrawArrays(GL_POINTS, 0, vertices_count);

	vao.Unbind();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Cube::GenerateData(int division, glm::vec3 start, float length)
{
	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;

	auto cps = calculateControlPoints(start, length, division);
	for (auto cp : cps) {
		vertices.push_back(cp.x);
		vertices.push_back(cp.y);
		vertices.push_back(cp.z);
	}

	// xy wireframe
	for (int i = 0; i < division; i++) {
		// z loop
		int zOffset = i * division * division;

		for (int j = 0; j < division; j++) {
			// y loop
			int yOffset = j * division;

			for (int k = 0; k < division; k++) {
				// x loop
				if (k > 1){
					indices.push_back(indices[indices.size() - 1]);
				}
				indices.push_back(zOffset + yOffset + k);
			}
		}
	}

	// zy wireframe
	for (int i = 0; i < division; i++) {
		// x loop
		int xOffset = i;

		for (int j = 0; j < division; j++) {
			// y loop
			int yOffset = j * division;

			for (int k = 0; k < division; k++) {
				// z loop
				if (k > 1) {
					indices.push_back(indices[indices.size() - 1]);
				}
				indices.push_back(xOffset + yOffset + k * division * division);
			}
		}
	}

	// yz wireframe
	for (int i = 0; i < division; i++) {
		// x loop
		int xOffset = i;

		for (int j = 0; j < division; j++) {
			// z loop
			int zOffset = j * division * division;

			for (int k = 0; k < division; k++) {
				// y loop
				if (k > 1) {
					indices.push_back(indices[indices.size() - 1]);
				}
				indices.push_back(zOffset + xOffset + k * division);
			}
		}
	}

	// bezier patches
	std::vector<GLuint> bezierIndices = {
		// front
		0, 1, 2, 3,			4, 5, 6, 7,			8, 9, 10, 11,		12, 13, 14, 15,
		// left
		48, 32, 16, 0, 		52, 36, 20, 4,		56, 40, 24, 8,		60, 44, 28, 12,
		// right
		3, 19, 35, 51,		7, 23, 39, 55,		11, 27, 43, 59,		15, 31, 47, 63,
		// back
		51, 50, 49, 48,		55, 54, 53, 52,		59, 58, 57, 56,		63, 62, 61, 60,
		// top
		12, 13, 14, 15,		28, 29, 30, 31,		44, 45, 46, 47,		60, 61, 62, 63,
		// bottom
		48, 49, 50, 51,		32, 33, 34, 35,		16, 17, 18, 19,		0, 1, 2, 3
	};

	indices.insert(indices.end(), bezierIndices.begin(), bezierIndices.end());

	return std::make_tuple(vertices, indices);
}

void Cube::SetControlPoints(std::vector<glm::vec3> cps)
{
	controlPoints = cps;

	std::vector<GLfloat> vertices;
	for (auto cp : controlPoints) {
		vertices.push_back(cp.x);
		vertices.push_back(cp.y);
		vertices.push_back(cp.z);
	}

	RefreshVertices(vertices);
}

std::vector<glm::vec3> Cube::GetCorners()
{
	std::vector<glm::vec3> corners;
	corners.push_back(controlPoints[0]);
	corners.push_back(controlPoints[3]);
	corners.push_back(controlPoints[15]);
	corners.push_back(controlPoints[12]);
	corners.push_back(controlPoints[48]);
	corners.push_back(controlPoints[51]);
	corners.push_back(controlPoints[63]);
	corners.push_back(controlPoints[60]);
	return corners;
}

std::vector<glm::vec3> Cube::calculateControlPoints(glm::vec3 start, float length, int division) const
{
	std::vector<glm::vec3> cps;

	float step = length / (division - 1);

	for (int i = 0; i < division; i++) {
		// z loop
		float z = start.z + i * step;

		for (int j = 0; j < division; j++) {
			// y loop
			float y = start.y + j * step;

			for (int k = 0; k < division; k++) {
				// x loop
				float x = start.x + k * step;

				cps.push_back(glm::vec3(x, y, z));
			}
		}
	}

	return cps;
}
