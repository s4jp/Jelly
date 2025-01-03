#include "model.h"
#include "Parser.h"

Model::Model(std::string path) 
	: Figure(Parser::ParseObj(path)) {
	// stupid ahh workaround
	FillMinMaxValues(path);
}

void Model::Render(int colorLoc)
{
	vao.Bind();

	glUniform4fv(colorLoc, 1, color);
	glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, 0);

	vao.Unbind();
}

void Model::FillMinMaxValues(std::string path)
{
	std::tuple<std::vector<VertexStruct>, std::vector<GLuint>> data = Parser::ParseObj(path);

	minMaxValues = {
		std::numeric_limits<float>::max(),		// xMin
		std::numeric_limits<float>::min(),		// xMax
		std::numeric_limits<float>::max(),		// yMin
		std::numeric_limits<float>::min(),		// yMax
		std::numeric_limits<float>::max(),		// zMin
		std::numeric_limits<float>::min(),		// zMax
	};

	std::vector<VertexStruct> vertices = std::get<0>(data);
	for (const auto& vertex : vertices) {
		minMaxValues[0] = std::min(minMaxValues[0], vertex.position.x);
		minMaxValues[1] = std::max(minMaxValues[1], vertex.position.x);
		minMaxValues[2] = std::min(minMaxValues[2], vertex.position.y);
		minMaxValues[3] = std::max(minMaxValues[3], vertex.position.y);
		minMaxValues[4] = std::min(minMaxValues[4], vertex.position.z);
		minMaxValues[5] = std::max(minMaxValues[5], vertex.position.z);
	}
}
