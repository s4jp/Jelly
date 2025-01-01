#include "cube.h"
#include <glm/gtc/type_ptr.hpp>

const glm::vec4 color = glm::vec4(1, 1, 1, 1);
const float cpSize = 5.f;

Cube::Cube(int division, glm::vec3 center, float length) 
	: Figure(GenerateData(division, center, length)) {}

void Cube::Render(int colorLoc)
{
	vao.Bind();

	glUniform4fv(colorLoc, 1, glm::value_ptr(color));
	glDrawElements(GL_LINES, indices_count, GL_UNSIGNED_INT, 0);

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

	float step = length / division;

	for (int i = 0; i < division; i++) {
		// z loop
		float z = start.z + i * step;

		for (int j = 0; j < division; j++){
			// y loop
			float y = start.y + j * step;

			for (int k = 0; k < division; k++) {
				// x loop
				float x = start.x + k * step;

				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);
			}
		}
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

	return std::make_tuple(vertices, indices);
}
