#include "model.h"
#include "Parser.h"

Model::Model(std::string path) 
	: Figure(Parser::ParseObj(path)){}

void Model::Render(int colorLoc)
{
	vao.Bind();

	glUniform4fv(colorLoc, 1, color);
	glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, 0);

	vao.Unbind();
}
